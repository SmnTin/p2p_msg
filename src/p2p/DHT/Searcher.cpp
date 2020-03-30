#include "p2p/DHT/Searcher.h"

#include "p2p/DHT/DHT.h"

namespace p2p {
    namespace DHT {

        Searcher::Searcher(DHT *dht) : _dht(dht) {

        }

        void Searcher::findNode(const NodeId &nodeId, NodeSearch::Callback callback) {
            std::lock_guard<std::recursive_mutex> guard(_searchesMutex);

            assert(callback);

            auto nodes = _dht->getRoutingTable()->findNearestNodes(
                    nodeId, MAX_NODES_IN_FIND_NODE_RESPONSE);
            NodeSearch search;
            search.nodesRequestedLastTime = nodes.size();
            search.nodes = nodes;
            search.callback = std::move(callback);
            if(!search.callback)
                throw;
            search.destinationNode = nodeId;
            Network::MessageCounter tid = Task::generateId();
            _nodesSearches[tid] = search;

            findNodeStep(tid);
        }

        void Searcher::onFindNodeTimeout(Network::MessageCounter tid) {
            std::lock_guard<std::recursive_mutex> guard(_searchesMutex);
            auto it = _nodesSearches.find(tid);

            if(it == _nodesSearches.end())
                return;

            auto & curSearch = it->second;
            std::cout << "Node searching timed out\n";

            if(curSearch.nodesResponsed >= ceil(it->second.nodesRequestedLastTime
                                                * FIND_NODE_TIMEOUT_THRESHOLD_TO_CONTINUE))
                findNodeStep(tid);
            else
                _nodesSearches.erase(it);
        }

        void Searcher::findNodeStep(Network::MessageCounter tid) {
            std::lock_guard<std::recursive_mutex> guard(_searchesMutex);
            auto it = _nodesSearches.find(tid);

            if(it == _nodesSearches.end())
                return;

            auto & curSearch = it->second;
            if(!curSearch.callback)
                throw;

            curSearch.iterations++;

            std::shared_ptr<Node> requestedNode = nullptr;

            std::cout << "Current nodes list used in search:\n";
            for(auto & node : curSearch.nodes)
                std::cout << node->id() << "\n" << node->address() << "\n";

            if(!curSearch.nodes.empty())
                requestedNode = curSearch.nodes[0];

            if((requestedNode && requestedNode->id() == curSearch.destinationNode) ||
               curSearch.iterations > MAX_FIND_NODE_ITERATIONS ||
               curSearch.nodes.empty()) {
                curSearch.task->cancel();

                curSearch.callback(requestedNode);
//                _routingTable->getSheduler()->createTask(std::chrono::milliseconds{0}, tid,
//                        std::bind(curSearch.callback, requestedNode));
                _nodesSearches.erase(it);

                return;
            }

            curSearch.nodesRequestedLastTime = 0;

            for(auto & node : curSearch.nodes) {
//                if(node->id() == _routingTable->getId() || node->address() == _routingTable->getNetworkEngine()->getThisEndpoint())
//                    continue;
                curSearch.nodesRequestedLastTime++;
                _dht->getNetworkEngine()->sendMessage(
                        std::make_shared<FindNodeRequestMessage>(
                                tid, _dht->getRoutingTable()->getId(), curSearch.destinationNode), node->address());
            }

            curSearch.nodes.clear();


            curSearch.task = _dht->getSheduler()->executeTaskIn(std::chrono::milliseconds{FIND_NODE_TIMEOUT_IN_MILLISECONDS},
                                                     std::bind(&Searcher::onFindNodeTimeout, this, tid), tid);

        }

        void Searcher::onFindNodeRequest(p2p::DHT::FindNodeRequestMessage &msg,
                                                  boost::asio::ip::udp::endpoint &endp) {
//            if(msg.getNodeId() == _routingTable->getId())
//                return;
            _dht->getNetworkEngine()->sendMessage(
                    std::make_shared<FindNodeResponseMessage>(
                            msg.getCounter(), _dht->getRoutingTable()->getId(),
                            _dht->getRoutingTable()->findNearestNodes(
                                    msg.getDestNodeId(), MAX_NODES_IN_FIND_NODE_RESPONSE)), endp);
        }

        void Searcher::onFindNodeResponse(p2p::DHT::FindNodeResponseMessage &msg,
                                                   boost::asio::ip::udp::endpoint &endp) {
//            if(msg.getNodeId() == _routingTable->getId())
//                return;
            std::lock_guard<std::recursive_mutex> guard(_searchesMutex);
            std::cout << "=addr " << msg.getNodeId() << "\n";
            Network::MessageCounter tid = msg.getCounter();
            if(_nodesSearches.find(tid) == _nodesSearches.end())
                return;
            auto & curSearch = _nodesSearches[tid];
            curSearch.nodesResponsed++;

            for(auto n : msg.getNodes()) {
//                if(n->id() == _routingTable->getId())
//                    continue;
                std::cout << "=cont " << n->id() << "\n";
                auto & ans = curSearch.nodes;
                auto & nodeId = curSearch.destinationNode;
                auto & myId = _dht->getRoutingTable()->getId();
                auto here = std::find_if(ans.begin(), ans.end(),
                                         [&nodeId,&n](std::shared_ptr<Node> &node) {
                                             return nodeId.xorCmp(n->id(), node->id()) < 0;
                                         }
                );
                ans.insert(here, n);
            }

            if(curSearch.nodes.size() > MAX_NODES_IN_FIND_NODE_RESPONSE) {
                curSearch.nodes.resize(MAX_NODES_IN_FIND_NODE_RESPONSE);
            }

            if(curSearch.nodesResponsed >= curSearch.nodesRequestedLastTime)
                findNodeStep(tid);

        }

        void Searcher::storeValue(const std::shared_ptr<Node> &node, const NodeId &key,
                                           const StorageValueBuffer &val) {
            std::lock_guard<std::recursive_mutex> guard(_storeValueRequestsMutex);

            Network::MessageCounter tid = Task::generateId();

            StoreValueRequest req;
            req.destinationNode = node;
            req.key = key;
            req.val = val;

            _storeValueRequests[tid] = req;

            onStoreValueTimeout(tid);
        }

        void Searcher::onStoreValueRequest(StoreValueRequestMessage &msg,
                                                    boost::asio::ip::udp::endpoint &endp) {
            std::lock_guard<std::recursive_mutex> guard(_storeValueRequestsMutex);
            std::lock_guard<std::recursive_mutex> guard2(_storageMutex);

            _storage[msg.getKey()] = msg.getVal();

            _dht->getNetworkEngine()->sendMessage(std::make_shared<StoreValueResponseMessage>(
                    msg.getCounter(), _dht->getRoutingTable()->getId(), msg.getKey()), endp);
        }

        void Searcher::onStoreValueResponse(StoreValueResponseMessage &msg,
                                                     boost::asio::ip::udp::endpoint &endp) {
            std::lock_guard<std::recursive_mutex> guard(_storeValueRequestsMutex);

            Network::MessageCounter tid = msg.getCounter();
            auto it = _storeValueRequests.find(tid);
            if(it == _storeValueRequests.end())
                return;

            it->second.task->cancel();
            _storeValueRequests.erase(it);
        }

        void Searcher::onStoreValueTimeout(Network::MessageCounter tid) {
            std::lock_guard<std::recursive_mutex> guard(_storeValueRequestsMutex);

            auto it = _storeValueRequests.find(tid);
            if(it == _storeValueRequests.end())
                return;
            auto & curReq = it->second;

            if(curReq.iterations < MAX_STORE_VALUE_REQUEST_ATTEMPTS) {
                curReq.iterations++;

                _dht->getNetworkEngine()->sendMessage(std::make_shared<StoreValueRequestMessage>(
                        tid, _dht->getRoutingTable()->getId(), curReq.key, curReq.val),
                                                               curReq.destinationNode->address());

                curReq.task = _dht->getSheduler()->executeTaskIn(std::chrono::milliseconds {STORE_VALUE_REQUEST_TIMEOUT_IN_MILLISECONDS},
                        std::bind(&Searcher::onStoreValueTimeout, this, tid), tid);
            } else {
                _storeValueRequests.erase(it);
            }
        }

        void Searcher::getValue(const std::shared_ptr<p2p::Node> &node, const NodeId &key,
                                         GetValueRequest::Callback callback) {
            std::lock_guard<std::recursive_mutex> guard(_getValueRequestsMutex);

            Network::MessageCounter tid = Task::generateId();

            GetValueRequest req;
            req.destinationNode = node;
            req.key = key;
            req.callback = std::move(callback);

            _getValueRequests[tid] = req;

            onGetValueTimeout(tid);
        }

        void Searcher::onGetValueRequest(GetValueRequestMessage &msg,
                                                  boost::asio::ip::udp::endpoint &endp) {
            std::lock_guard<std::recursive_mutex> guard(_getValueRequestsMutex);
            std::lock_guard<std::recursive_mutex> guard2(_storageMutex);

            StorageValueBuffer val;
            bool success;

            if(_storage.find(msg.getKey()) != _storage.end()) {
                success = true;
                val = _storage[msg.getKey()];
            } else {
                success = false;
            }

            _dht->getNetworkEngine()->sendMessage(std::make_shared<GetValueResponseMessage>(
                    msg.getCounter(), _dht->getRoutingTable()->getId(), msg.getKey(), val, success), endp);
        }

        void Searcher::onGetValueResponse(GetValueResponseMessage &msg,
                                                   boost::asio::ip::udp::endpoint &endp) {
            std::lock_guard<std::recursive_mutex> guard(_getValueRequestsMutex);

            Network::MessageCounter tid = msg.getCounter();
            auto it = _getValueRequests.find(tid);
            if(it == _getValueRequests.end())
                return;

            it->second.callback(msg.isSuccessful(), msg.getVal());

            it->second.task->cancel();
            _getValueRequests.erase(it);
        }

        void Searcher::onGetValueTimeout(Network::MessageCounter tid) {
            std::lock_guard<std::recursive_mutex> guard(_getValueRequestsMutex);

            auto it = _getValueRequests.find(tid);
            if(it == _getValueRequests.end())
                return;
            auto & curReq = it->second;

            if(curReq.iterations < MAX_GET_VALUE_REQUEST_ATTEMPTS) {
                curReq.iterations++;

                _dht->getNetworkEngine()->sendMessage(std::make_shared<GetValueRequestMessage>(
                        tid, _dht->getRoutingTable()->getId(), curReq.key), curReq.destinationNode->address());

                curReq.task = _dht->getSheduler()->executeTaskIn(std::chrono::milliseconds {GET_VALUE_REQUEST_TIMEOUT_IN_MILLISECONDS},
                        std::bind(&Searcher::onGetValueTimeout, this, tid), tid);
            } else {
                curReq.callback(false, StorageValueBuffer());
                _getValueRequests.erase(it);
            }
        }
    } //DHT
} //p2p