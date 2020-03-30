#ifndef P2P_MSG_DHT_H
#define P2P_MSG_DHT_H

#include "p2p/NodeId.h"
#include "p2p/DHT/DHTMessageHandlers.h"
#include "p2p/DHT/RoutingTable.h"
#include "p2p/DHT/Searcher.h"
#include "p2p/Network/NetworkEngine.h"
#include "p2p/Node.h"
#include "p2p/Sheduler.h"

#include <vector>
#include <utility>
#include <cmath>
#include <iostream>
#include <mutex>

namespace p2p {
    namespace DHT {

        class DHT {
        public:
            DHT(const std::shared_ptr<Network::NetworkEngine> & networkEngine,
                const std::shared_ptr<Sheduler> & sheduler);

            void pingNode(const boost::asio::ip::udp::endpoint & endp);
            void findNode(const NodeId & nodeId, const NodeSearch::Callback & callback);
            void storeValue(const NodeId &nodeId, const NodeId & key,
                    const StorageValueBuffer & val);
            void getValue(const NodeId &nodeId, const NodeId & key,
                    const GetValueRequest::Callback & callback);

            std::shared_ptr<Network::NetworkEngine> getNetworkEngine();
            std::shared_ptr<Sheduler> getSheduler();
            std::shared_ptr<RoutingTable> getRoutingTable();
            std::shared_ptr<Searcher> getSearcher();
            std::shared_ptr<DHTMessageHandler> getMessageHandler();

        private:
            std::shared_ptr<Network::NetworkEngine> _networkEngine;
            std::shared_ptr<DHTMessageHandler> _msgHandler;
            std::shared_ptr<Sheduler> _sheduler;
            std::shared_ptr<RoutingTable> _routingTable;
            std::shared_ptr<Searcher> _searcher;
        };
    }
};

#endif //P2P_MSG_DHT_H
