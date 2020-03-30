#ifndef P2P_MSG_NODESLIST_H
#define P2P_MSG_NODESLIST_H

#include "p2p/NodeId.h"
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

        class DHT;

        typedef std::list<std::shared_ptr<Node>> KBucket;
        typedef std::vector<KBucket> KBuckets;

        class RoutingTable {
        public:
            explicit RoutingTable(DHT *dht);

            ~RoutingTable();

            KBuckets::iterator findBucket(const NodeId &nodeId);

            bool putNode(std::shared_ptr<Node> node);

            KBucket::iterator contains(KBuckets::iterator &it, const NodeId &nodeId);

            bool splitBucket(KBuckets::iterator &it);

            std::vector<std::shared_ptr<Node>> findNearestNodes(const NodeId &nodeId, std::size_t count);

            const NodeId getId();

            void pingNode(const boost::asio::ip::udp::endpoint &endp);

            std::shared_ptr<Node> findNode(const NodeId &nodeId);

            friend std::ostream &operator<<(std::ostream &out, RoutingTable &table);

        private:
            void pingNodesTaskFunc();

            std::shared_ptr<Task> _pingNodesTask;

            DHT *_dht;

            NodeId _id;
            std::recursive_mutex _idMutex;

            KBuckets _buckets;
            std::recursive_mutex _bucketsMutex;

        };
    }
}
#endif //P2P_MSG_NODESLIST_H
