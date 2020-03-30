#include "p2p/DHT/DHT.h"

namespace p2p {
    namespace DHT {
        DHT::DHT(const std::shared_ptr <Network::NetworkEngine> & networkEngine,
                const std::shared_ptr <Sheduler> & sheduler)
                : _networkEngine(networkEngine), _sheduler(sheduler) {
            _msgHandler = std::make_shared<DHTMessageHandler> (this);
            _networkEngine->registerMessageHandler(_msgHandler);

            _routingTable = std::make_shared<RoutingTable> (this);
            _searcher = std::make_shared<Searcher> (this);
        }


        std::shared_ptr<Network::NetworkEngine> DHT::getNetworkEngine() {
            return _networkEngine;
        }

        std::shared_ptr<Sheduler> DHT::getSheduler() {
            return _sheduler;
        }

        std::shared_ptr<RoutingTable> DHT::getRoutingTable() {
            return _routingTable;
        }

        std::shared_ptr<Searcher> DHT::getSearcher() {
            return _searcher;
        }

        std::shared_ptr<DHTMessageHandler> DHT::getMessageHandler() {
            return _msgHandler;
        }

        void DHT::pingNode(const boost::asio::ip::udp::endpoint &endp) {
            _routingTable->pingNode(endp);
        }

        void DHT::findNode(const NodeId &nodeId, const NodeSearch::Callback & callback) {
            _searcher->findNode(nodeId, callback);
        }

        void DHT::storeValue(const NodeId &nodeId, const NodeId &key,
                             const StorageValueBuffer &val) {
            NodeSearch::Callback callback1 = [key, val, this] (std::shared_ptr<Node> node) {
                _searcher->storeValue(node, key, val);
            };
            _searcher->findNode(nodeId, callback1);
        }

        void DHT::getValue(const NodeId &nodeId, const NodeId &key,
                           const GetValueRequest::Callback & callback) {
            NodeSearch::Callback callback1 = [key, callback, this] (std::shared_ptr<Node> node) {
                _searcher->getValue(node, key, callback);
            };
            _searcher->findNode(nodeId, callback1);
        }
    } //DHT
} //p2p