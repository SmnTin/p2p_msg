#include "p2p/Basic/Network/Stream.h"

namespace p2p::Basic::Network {
    using namespace p2p::Network;

    Stream::Stream() {
        //opened by default in case there are no slave handshake
        _opened = true;
    }

    void Stream::append(IStreamPtr child) {
        child->setParent(shared_from_this());
        _children.emplace_back(std::move(child));
    }

    void Stream::setParent(IStreamPtr parent) {
        _parent = IStreamWPtr(parent);
    }

    //just passes it further
    void Stream::performHandshake() {
        for (auto &child : _children)
            child->performHandshake();
    }

    //just passes it further
    void Stream::performClosure() {
        size_t unclosedCnt = 0;
        for (auto &child : _children)
            if (child->opened() && child->needsToBeClosed() && !child->closed())
                unclosedCnt++;
        if (unclosedCnt == 0) {
            //actually close
            _closed = true;
            if (!_parent.expired())
                _parent.lock()->performClosure();
        }
    }

    bool Stream::opened() const {
        return _opened;
    }

    bool Stream::closed() const {
        return _closed;
    }

    bool Stream::needsToBeClosed() const {
        return false;
    }

    Subscription Stream::subscribe(MessageCallback callback) {
        return _publisher.subscribe(callback);
    }

    void Stream::receive(Buffer msg) {
        _publisher.publish(msg);
        for (auto &child : _children)
            child->receive(msg);
    }

    void Stream::send(Buffer msg) {
        if (!_parent.expired())
            _parent.lock()->send(msg);
    }
}