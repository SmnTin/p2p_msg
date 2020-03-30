#include "p2p/Network/RUDP/RUDPMessageHandlers.h"
#include "p2p/Network/RUDP/Engine.h"
#include "p2p/DHT/DHTMessages.h"

namespace p2p {
    namespace Network {
        namespace RUDP {

            RUDPMessageHandlers::RUDPMessageHandlers(Engine *engine) : _engine(engine) {}

            void RUDPMessageHandlers::parseMessage(const MessageBuffer &msgBuf, std::size_t bytesReceived,
                                                   boost::asio::ip::udp::endpoint &endp) {
                if((Message::getInt<MessageFlags>(msgBuf.begin()) & DHT::DHT_MESSAGES_FLAG) > 0)
                    return;

                auto type = Message::getInt<MessageType>(
                        std::next(msgBuf.begin(), sizeof(MessageFlags)));

                std::shared_ptr<RUDPMessage> anyMsg;

                switch(type) {
                    case KEEP_ALIVE_MESSAGE_TYPE: {
                        _onMessageKind<KeepAliveMessage, &RUDPMessageHandlers::_onKeepAlive>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case OPEN_CONNECTION_REQUEST_MESSAGE_TYPE: {
                        _onMessageKind<OpenConnectionRequestMessage, &RUDPMessageHandlers::_onOpenConnectionRequest>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case OPEN_CONNECTION_RESPONSE_MESSAGE_TYPE: {
                        _onMessageKind<OpenConnectionResponseMessage, &RUDPMessageHandlers::_onOpenConnectionResponse>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case CLOSE_CONNECTION_MESSAGE_TYPE: {
                        _onMessageKind<CloseConnectionMessage, &RUDPMessageHandlers::_onCloseConnection>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case SEQUENCE_END_REQUEST_MESSAGE_TYPE: {
                        _onMessageKind<SequenceEndRequestMessage, &RUDPMessageHandlers::_onSequenceEndRequest>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case SEQUENCE_END_RESPONSE_MESSAGE_TYPE: {
                        _onMessageKind<SequenceEndResponseMessage, &RUDPMessageHandlers::_onSequenceEndResponse>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case ACKNOWLEDGE_REQUEST_MESSAGE_TYPE: {
                        _onMessageKind<AcknowledgeRequestMessage, &RUDPMessageHandlers::_onAcknowledgeRequest>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case ACKNOWLEDGE_RESPONSE_MESSAGE_TYPE: {
                        _onMessageKind<AcknowledgeResponseMessage, &RUDPMessageHandlers::_onAcknowledgeResponse>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    case BLOCK_MESSAGE_TYPE: {
                        _onMessageKind<BlockMessage, &RUDPMessageHandlers::_onBlock>(msgBuf, bytesReceived, endp, anyMsg);
                        break;
                    }
                    default:
                        return;

                }
            }

            template<class T, void (RUDPMessageHandlers::*F)(T &, boost::asio::ip::udp::endpoint &)>
            void RUDPMessageHandlers::_onMessageKind(const MessageBuffer &msgBuf, std::size_t bytesReceived,
                    boost::asio::ip::udp::endpoint &endp, std::shared_ptr<RUDPMessage> &anyMsg) {
                T msg;
                msg.parse(msgBuf);

//                if(msg.getLength() != bytesReceived) {
//                    std::cout << "HMM... " << msg.getLength() << " " << bytesReceived << "\n";
//                    return;
//                }

                anyMsg = std::make_shared<T>(msg);
                (this->*F)(msg, endp);
            }

            void RUDPMessageHandlers::_onKeepAlive(KeepAliveMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
            }

            void RUDPMessageHandlers::_onOpenConnectionRequest(OpenConnectionRequestMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = _engine->findConnectionByNodeId(msg.getNodeId());
                if(conn == nullptr) {
                    conn = _engine->createConnection(std::make_shared<Node>(msg.getNodeId(), endp));
                    conn->acceptConnection(msg.getConnectionId());
                }
                _engine->getNetworkEngine()->sendMessage(std::make_shared<OpenConnectionResponseMessage> (
                        msg.getConnectionId(), conn->getConnectionId()), endp);
            }

            void RUDPMessageHandlers::_onOpenConnectionResponse(OpenConnectionResponseMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
                conn->onResponseMessage();
                conn->acceptConnection(msg.getEndpointConnetionId());
                conn->onConnectionResponse();
            }

            void RUDPMessageHandlers::_onCloseConnection(CloseConnectionMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
                conn->onResponseMessage();
                conn->onDisconnection();
                _engine->onDisconnection(conn);
            }

            void RUDPMessageHandlers::_onSequenceEndRequest(SequenceEndRequestMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
                conn->onSequenceEndRequest(msg.isTerminal());
            }

            void RUDPMessageHandlers::_onSequenceEndResponse(SequenceEndResponseMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
                conn->onResponseMessage();
                conn->onSequenceEndResponse();
            }

            void RUDPMessageHandlers::_onAcknowledgeRequest(AcknowledgeRequestMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
                conn->onAckRequest();
            }

            void RUDPMessageHandlers::_onAcknowledgeResponse(AcknowledgeResponseMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
                conn->onResponseMessage();
                conn->onAckResponse(msg.getReceivedBlocks());
            }

            void RUDPMessageHandlers::_onBlock(BlockMessage &msg, boost::asio::ip::udp::endpoint &endp) {
                auto conn = (*_engine)[msg.getConnectionId()];
                if(conn == nullptr)
                    return;
                conn->onEachMessage(endp);
                conn->onBlock(msg.getBlockIndex(), msg.getVal(), msg.getValLen());
            }

        } //RUDP
    } //Network
} //p2p