#ifndef P2P_MSG_RUDPMESSAGEHANDLERS_H
#define P2P_MSG_RUDPMESSAGEHANDLERS_H

#include "p2p/Network/MessageHandler.h"
#include "p2p/Network/RUDP/RUDPMessages.h"

namespace p2p {
    namespace Network {
        namespace RUDP {

            class Engine;

            class RUDPMessageHandlers : public MessageHandler {
            public:
                explicit RUDPMessageHandlers(Engine * engine);

                void parseMessage(const MessageBuffer & msgBuf, std::size_t bytesReceived, boost::asio::ip::udp::endpoint & endp);

            private:
                Engine * _engine;

                void _onKeepAlive              (KeepAliveMessage &msg,              boost::asio::ip::udp::endpoint &endp);
                void _onOpenConnectionRequest  (OpenConnectionRequestMessage &msg,  boost::asio::ip::udp::endpoint &endp);
                void _onOpenConnectionResponse (OpenConnectionResponseMessage &msg, boost::asio::ip::udp::endpoint &endp);
                void _onCloseConnection        (CloseConnectionMessage &msg,        boost::asio::ip::udp::endpoint &endp);
                void _onSequenceEndRequest     (SequenceEndRequestMessage &msg,     boost::asio::ip::udp::endpoint &endp);
                void _onSequenceEndResponse    (SequenceEndResponseMessage &msg,    boost::asio::ip::udp::endpoint &endp);
                void _onAcknowledgeRequest     (AcknowledgeRequestMessage &msg,     boost::asio::ip::udp::endpoint &endp);
                void _onAcknowledgeResponse    (AcknowledgeResponseMessage &msg,    boost::asio::ip::udp::endpoint &endp);
                void _onBlock                  (BlockMessage &msg,                  boost::asio::ip::udp::endpoint &endp);

                template<typename T, void (RUDPMessageHandlers::*F)(T &, boost::asio::ip::udp::endpoint &)>
                void _onMessageKind(const MessageBuffer &msgBuf, std::size_t bytesReceived,
                                   boost::asio::ip::udp::endpoint &endp, std::shared_ptr<RUDPMessage> & anyMsg);
            };
        } //RUDP
    } //Network
} //p2p

#endif //P2P_MSG_RUDPMESSAGEHANDLERS_H
