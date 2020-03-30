#ifndef P2P_MSG_DHTMESSAGEHANDLERS_H
#define P2P_MSG_DHTMESSAGEHANDLERS_H

#include <p2p/Node.h>
#include "p2p/Network/MessageHandler.h"
#include "p2p/DHT/DHTMessages.h"
#include "p2p/DHT/RoutingTable.h"
#include "p2p/DHT/Searcher.h"

#include <vector>
#include <utility>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace p2p {
    namespace DHT {

        class DHT;

        class DHTMessageHandler : public Network::MessageHandler {
        public:
            explicit DHTMessageHandler(DHT * dht);

            void parseMessage (const Network::MessageBuffer & msgBuf, std::size_t bytesReceived, boost::asio::ip::udp::endpoint & endp) override;

            void pingNode(const boost::asio::ip::udp::endpoint & endp);
        private:
            DHT * _dht;

            void onPing(PingMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onPong(PongMessage & msg, boost::asio::ip::udp::endpoint & endp);

            void onStoreValueRequest(StoreValueRequestMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onStoreValueResponse(StoreValueResponseMessage & msg, boost::asio::ip::udp::endpoint & endp);

            void onGetValueRequest(GetValueRequestMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onGetValueResponse(GetValueResponseMessage & msg, boost::asio::ip::udp::endpoint & endp);

            void onFindNodeRequest(FindNodeRequestMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onFindNodeResponse(FindNodeResponseMessage & msg, boost::asio::ip::udp::endpoint & endp);

            void onEachMessage(std::shared_ptr<Node> node);

            friend std::ostream & operator<< (std::ostream & out, RoutingTable & table);
        };
    } //DHT
} //p2p

#endif //P2P_MSG_DHTMESSAGEHANDLERS_H
