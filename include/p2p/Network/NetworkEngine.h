#ifndef P2P_MSG_NETWORKENGINE_H
#define P2P_MSG_NETWORKENGINE_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <vector>
#include <list>
#include <mutex>

#include "p2p/Buffer.h"
#include "p2p/Network/MessageHandler.h"
#include "p2p/Network/Message.h"

namespace p2p {
    namespace Network {

        class NetworkEngine {
        public:
            NetworkEngine(boost::asio::io_service &io, uint16_t port);

            void registerMessageHandler(std::weak_ptr<MessageHandler> msgHandler);

            void sendMessage(std::shared_ptr<LightMessage> msg, boost::asio::ip::udp::endpoint to);

            MessageCounter generateToken();

            boost::asio::ip::udp::endpoint getThisEndpoint();

            void clean();

        private:
            boost::asio::io_service &_io;
            boost::asio::ip::udp::socket _socket;
            boost::asio::ip::udp::endpoint _thisEndpoint;
            std::recursive_mutex _socketMutex;

            MessageBuffer _msgBuff;
            boost::asio::ip::udp::endpoint _remoteEndpoint;

            std::vector<std::weak_ptr<MessageHandler>> _msgHandlers;
            std::recursive_mutex _msgHandlersMutex;

            void _handleRecieve(const boost::system::error_code & err, std::size_t bytesReceived);
            void _handleSend(std::shared_ptr<MessageBuffer> /*message*/,
                             const boost::system::error_code& /*error*/,
                             std::size_t /*bytes_transferred*/,
                             const boost::asio::ip::udp::endpoint /*to*/);

            void _startRecieving();

            MessageCounter _counter;
            std::recursive_mutex _counterMutex;
        };
    }
}
#endif //P2P_MSG_NETWORKENGINE_H
