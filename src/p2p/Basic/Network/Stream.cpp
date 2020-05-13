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
    void Stream<MPolicy>::setChild(IStreamPtr child) {
        if (_child)
            _child->setParent(nullptr);
        if (child)
            child->setParent(shared_from_this());
        _child = (std::move(child));
    }

    template<class MPolicy>
    void Stream<MPolicy>::setChild(std::nullptr_t child) {
        setChild(IStreamPtr(child));
    }

    template<class MPolicy>
    void Stream<MPolicy>::setParent(IStreamPtr parent) {
        _parent = IStreamWPtr(parent);
    }

    template<class MPolicy>
    void Stream<MPolicy>::setParent(std::nullptr_t parent) {
        _parent = IStreamWPtr();
    }

    //just passes it further
    template<class MPolicy>
    void Stream<MPolicy>::performHandshake() {
        if (_child)
            _child->performHandshake();
    }

    //just passes it further
    template<class MPolicy>
    void Stream<MPolicy>::performClosure() {
        size_t unclosedCnt = 0;
        if (_child && _child->opened() && !_child->closed())
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
        if (_child && _child != prev)
            _child->close(shared_from_this());
        //this node is a leaf and must initiate the closure
        if (!_child)
            performClosure();
    }

    template<class MPolicy>
    void Stream<MPolicy>::send(Buffer msg) {
        if (!_parent.expired())
            _parent.lock()->send(msg);
    }

    template<class MPolicy>
    void Stream<MPolicy>::receive(Buffer msg) {
        MPolicy::spreadMessage(msg);
        if (_child)
            _child->receive(msg);
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

    template<class MPolicy>
    Endpoint Stream<MPolicy>::getEndpoint() const {
        if (!_endpoint.has_value())
            if (auto parent = _parent.lock())
                return parent->getEndpoint();
        return _endpoint.value();
    }

    template<class MPolicy>
    Endpoint Stream<MPolicy>::getEndpoint() {
        if (!_endpoint.has_value())
            if (auto parent = _parent.lock())
                _endpoint = parent->getEndpoint();
        return _endpoint.value();
    }

    template<class MPolicy>
    TransportTraits Stream<MPolicy>::getTraits() const {
        if (!_transportTraits.has_value())
            if (auto parent = _parent.lock())
                return parent->getTraits();
        return _transportTraits.value();
    }

    template<class MPolicy>
    TransportTraits Stream<MPolicy>::getTraits() {
        if (!_transportTraits.has_value())
            if (auto parent = _parent.lock())
                _transportTraits = parent->getTraits();
        return _transportTraits.value();
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