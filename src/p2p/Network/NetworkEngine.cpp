
#include "p2p/Network/NetworkEngine.h"

#include <iostream>

namespace p2p {
    namespace Network {

        NetworkEngine::NetworkEngine(boost::asio::io_service &io, uint16_t port)
                : _io(io), _thisEndpoint(boost::asio::ip::udp::v4(), port),
                _socket(io, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)) {
//            _socket.non_blocking(true);
            _startRecieving();
        }

        void NetworkEngine::_startRecieving() {
            std::lock_guard<std::recursive_mutex> guard(_socketMutex);

            _socket.async_receive_from(
                    boost::asio::buffer(_msgBuff), _remoteEndpoint,
                    boost::bind(&NetworkEngine::_handleRecieve, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
        }

        void NetworkEngine::_handleRecieve(const boost::system::error_code &err, std::size_t bytesReceived) {
//            std::cout << "Received message with error code " << err << " from " << _remoteEndpoint << "\n";
            if(!err) {
                std::lock_guard<std::recursive_mutex> guard(_msgHandlersMutex);
                for(auto & ptr : _msgHandlers)
                    ptr.lock()->parseMessage(_msgBuff, bytesReceived, _remoteEndpoint);
            }
            _startRecieving();
        }

        void NetworkEngine::_handleSend(std::shared_ptr<MessageBuffer>, const boost::system::error_code & err,
                                        std::size_t bytesSent, const boost::asio::ip::udp::endpoint to) {
//            std::cout << "Sent " << bytesSent << " bytes with error code " << err << " to " << to << "\n";
        }

        void NetworkEngine::registerMessageHandler(std::weak_ptr<p2p::Network::MessageHandler> msgHandler) {
            std::lock_guard<std::recursive_mutex> guard(_msgHandlersMutex);
            _msgHandlers.push_back(msgHandler);
        }

        void NetworkEngine::sendMessage(std::shared_ptr<LightMessage> msg, boost::asio::ip::udp::endpoint to) {
            std::lock_guard<std::recursive_mutex> guard(_socketMutex);

            _socket.async_send_to(boost::asio::buffer(*(msg->getData()), msg->getLength()), to,
                      boost::bind(&NetworkEngine::_handleSend, this, msg->getData(),
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred,
                                  to));
        }

        MessageCounter NetworkEngine::generateToken() {
            std::lock_guard<std::recursive_mutex> guard(_counterMutex);

            return _counter++;
        }

        boost::asio::ip::udp::endpoint NetworkEngine::getThisEndpoint() {
            return _thisEndpoint;
        }
    }
}
