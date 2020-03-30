#ifndef P2P_MSG_DHTSEARCHER_H
#define P2P_MSG_DHTSEARCHER_H

#include "p2p/Node.h"
#include "p2p/Network/MessageHandler.h"
#include "p2p/DHT/DHTMessages.h"
#include "p2p/Sheduler.h"

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

        struct NodeSearch {
            std::vector<std::shared_ptr<Node>> nodes;
            NodeId destinationNode;
            int nodesRequestedLastTime = 0;
            int nodesResponsed = 0;
            int iterations = 0;
            std::shared_ptr<Task> task;
            typedef std::function<void(std::shared_ptr<Node>)> Callback;
            Callback callback;
        };

        struct StoreValueRequest {
            std::shared_ptr<Node> destinationNode;
            NodeId key;
            StorageValueBuffer val = StorageValueBuffer();
            std::shared_ptr<Task> task;
            int iterations = 0;
        };

        struct GetValueRequest {
            std::shared_ptr<Node> destinationNode;
            NodeId key;
            int iterations = 0;
            std::shared_ptr<Task> task;
            typedef std::function<void(bool, StorageValueBuffer)> Callback;
            Callback callback;
        };

        class Searcher {
        public:
            explicit Searcher(DHT * dht);

            void findNode(const NodeId & nodeId, NodeSearch::Callback callback);
            void storeValue(const std::shared_ptr<Node> &node, const NodeId & key, const StorageValueBuffer & val);
            void getValue(const std::shared_ptr<Node> &node, const NodeId & key, GetValueRequest::Callback callback);

            friend class DHTMessageHandler;

        private:
            void onStoreValueRequest(StoreValueRequestMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onStoreValueResponse(StoreValueResponseMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onStoreValueTimeout(Network::MessageCounter tid);

            void onGetValueRequest(GetValueRequestMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onGetValueResponse(GetValueResponseMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onGetValueTimeout(Network::MessageCounter tid);

            void onFindNodeRequest(FindNodeRequestMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onFindNodeResponse(FindNodeResponseMessage & msg, boost::asio::ip::udp::endpoint & endp);
            void onFindNodeTimeout(Network::MessageCounter tid);
            void findNodeStep(Network::MessageCounter tid);

            DHT * _dht;

            std::unordered_map<Task::Id, NodeSearch> _nodesSearches;
            std::recursive_mutex _searchesMutex;

            std::unordered_map<Task::Id, StoreValueRequest> _storeValueRequests;
            std::recursive_mutex _storeValueRequestsMutex;

            std::unordered_map<Task::Id, GetValueRequest> _getValueRequests;
            std::recursive_mutex _getValueRequestsMutex;

            std::unordered_map<NodeId, StorageValueBuffer> _storage;
            std::recursive_mutex _storageMutex;
        };
    }
}
#endif //P2P_MSG_DHTSEARCHER_H
