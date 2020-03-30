#include "p2p/DHT/DHTMessageHandlers.h"

#include "p2p/DHT/DHT.h"

namespace p2p {
    namespace DHT {

        DHTMessageHandler::DHTMessageHandler(DHT * dht)
                : _dht(dht) {

        }

        void DHTMessageHandler::parseMessage(const p2p::Network::MessageBuffer &msgBuf, std::size_t bytesReceived,
                                                boost::asio::ip::udp::endpoint &endp) {
            if((Network::Message::getInt<Network::MessageFlags>(msgBuf.begin()) & DHT_MESSAGES_FLAG) == 0)
                return;

            auto type = Network::Message::getInt<Network::MessageType>(
                    std::next(msgBuf.begin(), sizeof(Network::MessageFlags)));

            std::shared_ptr<Network::Message> anyMsg;

            switch (type) {
                case(PING_MESSAGE_TYPE): {
                    PingMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    anyMsg = std::make_shared<PingMessage>(msg);
                    onPing(msg, endp);
                    break;
                }
                case(PONG_MESSAGE_TYPE): {
                    PongMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    anyMsg = std::make_shared<PongMessage>(msg);
                    onPong(msg, endp);
                    break;
                }
                case(FIND_NODE_REQUEST_MESSAGE_TYPE): {
                    FindNodeRequestMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    onFindNodeRequest(msg, endp);
                    anyMsg = std::make_shared<FindNodeRequestMessage>(msg);
                    break;
                }
                case(FIND_NODE_RESPONSE_MESSAGE_TYPE): {
                    FindNodeResponseMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    onFindNodeResponse(msg, endp);
                    anyMsg = std::make_shared<FindNodeResponseMessage>(msg);
                    break;
                }
                case(STORE_VALUE_REQUEST_MESSAGE_TYPE): {
                    StoreValueRequestMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    onStoreValueRequest(msg, endp);
                    anyMsg = std::make_shared<StoreValueRequestMessage>(msg);
                    break;
                }
                case(STORE_VALUE_RESPONSE_MESSAGE_TYPE): {
                    StoreValueResponseMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    onStoreValueResponse(msg, endp);
                    anyMsg = std::make_shared<StoreValueResponseMessage>(msg);
                    break;
                }
                case(GET_VALUE_REQUEST_MESSAGE_TYPE): {
                    GetValueRequestMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    onGetValueRequest(msg, endp);
                    anyMsg = std::make_shared<GetValueRequestMessage>(msg);
                    break;
                }
                case(GET_VALUE_RESPONSE_MESSAGE_TYPE): {
                    GetValueResponseMessage msg;
                    msg.parse(msgBuf);
                    if (msg.getLength() != bytesReceived)
                        return;
                    onGetValueResponse(msg, endp);
                    anyMsg = std::make_shared<GetValueResponseMessage>(msg);
                    break;
                }
                default:
                    return;
            }

            onEachMessage(std::make_shared<Node>(anyMsg->getNodeId(), endp));
        }

        void DHTMessageHandler::onStoreValueRequest(StoreValueRequestMessage &msg,
                                           boost::asio::ip::udp::endpoint &endp) {
            _dht->getSearcher()->onStoreValueRequest(msg, endp);
        }

        void DHTMessageHandler::onStoreValueResponse(StoreValueResponseMessage &msg,
                                            boost::asio::ip::udp::endpoint &endp) {
            _dht->getSearcher()->onStoreValueResponse(msg, endp);
        }

        void DHTMessageHandler::onGetValueRequest(GetValueRequestMessage &msg,
                                         boost::asio::ip::udp::endpoint &endp) {
            _dht->getSearcher()->onGetValueRequest(msg, endp);
        }

        void DHTMessageHandler::onGetValueResponse(GetValueResponseMessage &msg,
                                          boost::asio::ip::udp::endpoint &endp) {
            _dht->getSearcher()->onGetValueResponse(msg, endp);
        }

        void DHTMessageHandler::onFindNodeRequest(p2p::DHT::FindNodeRequestMessage &msg,
                                                  boost::asio::ip::udp::endpoint &endp) {
            _dht->getSearcher()->onFindNodeRequest(msg, endp);
        }

        void DHTMessageHandler::onFindNodeResponse(p2p::DHT::FindNodeResponseMessage &msg,
                                                   boost::asio::ip::udp::endpoint &endp) {
            _dht->getSearcher()->onFindNodeResponse(msg, endp);
        }

        void DHTMessageHandler::onPing(PingMessage &msg, boost::asio::ip::udp::endpoint &endp) {
//            std::cout << "Received ping message from " << endp << "\n";
            _dht->getNetworkEngine()->sendMessage(
                    std::make_shared<PongMessage>(msg.getCounter(), _dht->getRoutingTable()->getId()), endp);
        }

        void DHTMessageHandler::onPong(PongMessage &msg, boost::asio::ip::udp::endpoint &endp) {
//            std::cout << "Received pong message from " << endp << "\n";
        }

        void DHTMessageHandler::pingNode(const boost::asio::ip::udp::endpoint &endp) {
            _dht->getNetworkEngine()->sendMessage(
                    std::make_shared<PingMessage>(_dht->getNetworkEngine()->generateToken(),
                                                  _dht->getRoutingTable()->getId()), endp);
        }

        void DHTMessageHandler::onEachMessage(std::shared_ptr<p2p::Node> node) {
            _dht->getRoutingTable()->putNode(std::move(node));
        }

    } //DHT
} //p2p