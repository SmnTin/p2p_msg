#ifndef P2P_MSG_MESSAGEHANDLER_H
#define P2P_MSG_MESSAGEHANDLER_H

#include <memory>
#include <boost/asio.hpp>

#include "p2p/config.h"
#include "p2p/Buffer.h"

namespace p2p {
    namespace Network {

        class MessageHandler {
        public:
            virtual void parseMessage(const MessageBuffer & msgBuf, std::size_t bytesReceived, boost::asio::ip::udp::endpoint & endp) = 0;

            virtual ~MessageHandler() {};
        };
    }
}

#endif //P2P_MSG_MESSAGEHANDLER_H
