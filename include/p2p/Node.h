#ifndef P2P_MSG_NODE_H
#define P2P_MSG_NODE_H

#include "p2p/NodeId.h"

#include <boost/asio.hpp>
#include <iostream>

namespace p2p {

    class Node {
    public:
        Node(const NodeId &id_, const boost::asio::ip::udp::endpoint &address_)
                : _id(id_), _address(address_) {}

        ~Node() = default;

        NodeId & id() {
            return _id;
        }
        boost::asio::ip::udp::endpoint & address() {
            return _address;
        }

        bool isGood(time_t t) {
            return t <= _lastTimeHeard + MAX_UNANSWERED_TIME_IN_SECONDS;
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
        boost::asio::ip::udp::endpoint _address;

        bool _expired = false;

        time_t _lastTimeHeard = time(0);
    };
}

#endif //P2P_MSG_NODE_H
