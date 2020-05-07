#include "p2p/Basic/Network/Stream.h"

namespace p2p::Basic::Network {
    using namespace p2p::Network;
    using namespace p2p;

    template<class MPolicy>
    Stream<MPolicy>::Stream() {
        //opened by default in case there are no slave handshake
        _opened = true;
    }

    template<class MPolicy>
    void Stream<MPolicy>::append(IStreamPtr child) {
        child->setParent(shared_from_this());
        _children.emplace_back(std::move(child));
        recalcClosureNecessity();
    }

    template<class MPolicy>
    void Stream<MPolicy>::setParent(IStreamPtr parent) {
        _parent = IStreamWPtr(parent);
    }

    //just passes it further
    template<class MPolicy>
    void Stream<MPolicy>::performHandshake() {
        for (auto &child : _children)
            child->performHandshake();
    }

    //just passes it further
    template<class MPolicy>
    void Stream<MPolicy>::performClosure() {
        size_t unclosedCnt = 0;
        for (auto &child : _children)
            if (child->opened() && child->subtreeNeedsToBeClosed() && !child->closed())
                unclosedCnt++;
        if (unclosedCnt == 0) {
            //actually close
            _closed = true;
            if (!_parent.expired())
                _parent.lock()->performClosure();
        }
    }

    template<class MPolicy>
    bool Stream<MPolicy>::opened() const {
        return _opened;
    }

    template<class MPolicy>
    bool Stream<MPolicy>::closed() const {
        return _closed;
    }

    template<class MPolicy>
    void Stream<MPolicy>::close(IStreamPtr prev) {
        if (auto parent = _parent.lock())
            if (parent != prev)
                parent->close(shared_from_this());
        for (auto &child : _children)
            if (child != prev)
                child->close(shared_from_this());
        //this node is a leaf and must initiate the closure
        if (_children.empty())
            performClosure();
    }

    template<class MPolicy>
    bool Stream<MPolicy>::subtreeNeedsToBeClosed() const {
        return _subtreeNeedsToBeClosed;
    }

    template<typename MPolicy>
    void Stream<MPolicy>::recalcClosureNecessity() {
        bool ans = _needsToBeClosed;
        for (auto &child : _children)
            ans = (ans || child->subtreeNeedsToBeClosed());
        _subtreeNeedsToBeClosed = ans;
        if (auto parent = _parent.lock())
            if (parent->subtreeNeedsToBeClosed() != ans) //gives amortized O(1)
                parent->recalcClosureNecessity();
    }

    template<class MPolicy>
    void Stream<MPolicy>::setClosureNecessity(bool flag) {
        _needsToBeClosed = flag;
        recalcClosureNecessity();
    }

    template<class MPolicy>
    void Stream<MPolicy>::send(Buffer msg) {
        if (!_parent.expired())
            _parent.lock()->send(msg);
    }

    template<class MPolicy>
    void Stream<MPolicy>::receive(Buffer msg) {
        MPolicy::spreadMessage(msg);
        for (auto &child : _children)
            child->receive(msg);
    }

    template<class MPolicy>
    NodeId Stream<MPolicy>::getNodeId() const {
        if (!_nodeId.has_value())
            if (auto parent = _parent.lock())
                return parent->getNodeId();
        return _nodeId.value();
    }

    template<class MPolicy>
    NodeId Stream<MPolicy>::getNodeId() {
        if (!_nodeId.has_value())
            if (auto parent = _parent.lock())
                _nodeId = parent->getNodeId();
        return _nodeId.value();
    }

    void Messaging::NoPolicy::spreadMessage
            (Buffer msg) {}

    void Messaging::SubscriptionPolicy::spreadMessage
            (Buffer msg) {
        _publisher.publish(msg);
    }

    void Messaging::QueuePolicy::spreadMessage
            (Buffer msg) {
        _msgQueue.push(msg);
    }

    Subscription Messaging::SubscriptionPolicy::subscribe
            (MessageCallback callback) {
        return _publisher.subscribe(callback);
    }

    bool Messaging::QueuePolicy::available() const {
        return !_msgQueue.empty();
    }

    Buffer Messaging::QueuePolicy::read() {
        if (!available())
            throw std::runtime_error("No messages are in the queue!");
        auto msg = _msgQueue.front();
        _msgQueue.pop();
        return msg;
    }

    template class Stream<Messaging::NoPolicy>;
    template class Stream<Messaging::SubscriptionPolicy>;
    template class Stream<Messaging::QueuePolicy>;
}