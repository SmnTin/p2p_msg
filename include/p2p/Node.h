#ifndef P2P_MSG_NODE_H
#define P2P_MSG_NODE_H

#include "p2p/NodeId.h"

#include <boost/asio.hpp>
#include <iostream>
#include <memory>

namespace p2p {

    class Node {
    public:
        Node(const NodeId &id_)
                : _id(id_) {}

        ~Node() = default;

        NodeId & id() {
            return _id;
        }

        bool isGood(time_t t) {
            return t <= _lastTimeHeard + 100;
        }

        void setExpired() {
            _expired = true;
        }
        bool isExpired() {
            return _expired;
        }

        void heard(time_t t) {
            _lastTimeHeard = t;
        }

    private:
        NodeId _id;
        bool _expired = false;

        time_t _lastTimeHeard = time(0);
    };

    typedef std::shared_ptr<Node> NodePtr;
}

#endif //P2P_MSG_NODE_H
