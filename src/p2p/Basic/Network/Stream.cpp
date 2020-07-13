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
        _child = (std::move(child));
    }

    template<class MPolicy>
    void Stream<MPolicy>::append(IStreamPtr child) {
        if (_child)
            _child->setParent(nullptr);
        if (child)
            child->setParent(shared_from_this());
        setChild(child);
    }

    template<class MPolicy>
    void Stream<MPolicy>::setParent(IStreamPtr parent) {
        if (parent)
            _parent = IStreamWPtr(parent);
        else
            _parent = IStreamWPtr();
    }

    //just passes it further
    template<class MPolicy>
    void Stream<MPolicy>::performHandshake() {
        _opened = true;
        if (_child)
            _child->performHandshake();
        else
            reportThatOpened();
    }

    //just passes it further
    template<class MPolicy>
    void Stream<MPolicy>::performClosure() {
        _closed = true;
        if (auto parent = _parent.lock())
            parent->performClosure();
    }

    template<class MPolicy>
    void Stream<MPolicy>::reportThatOpened() {
        if (auto parent = _parent.lock())
            parent->reportThatOpened();
    }

    template<class MPolicy>
    bool Stream<MPolicy>::opened() {
        return _opened;
    }

    template<class MPolicy>
    bool Stream<MPolicy>::closed() {
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
            _parent.lock()->send(std::move(msg));
    }

    template<class MPolicy>
    void Stream<MPolicy>::receive(Buffer msg) {
        if (_child) {
            MPolicy::spreadMessage(msg);
            _child->receive(std::move(msg));
        } else {
            MPolicy::spreadMessage(std::move(msg));
        }
    }

    template<class MPolicy>
    NodeId Stream<MPolicy>::getNodeId() {
        if (!_nodeId.has_value())
            if (auto parent = _parent.lock())
                _nodeId = parent->getNodeId();
        return _nodeId.value();
    }

    template<class MPolicy>
    Endpoint Stream<MPolicy>::getEndpoint() {
        if (!_endpoint.has_value())
            if (auto parent = _parent.lock())
                _endpoint = parent->getEndpoint();
        return _endpoint.value();
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