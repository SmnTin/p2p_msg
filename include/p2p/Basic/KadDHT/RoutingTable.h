#ifndef P2P_MSG_NODESLIST_H
#define P2P_MSG_NODESLIST_H

#include "p2p/NodeId.h"
#include "p2p/Node.h"
#include "p2p/Sheduler.h"
#include "p2p/Host/HostId.h"

#include <vector>
#include <utility>
#include <cmath>
#include <iostream>

namespace p2p::Basic::KadDHT {

    typedef std::list<NodePtr> KBucket;
    typedef std::vector<KBucket> KBuckets;

    class RoutingTable {
    public:
        explicit RoutingTable(std::shared_ptr<HostId> hostId);

        ~RoutingTable();

        KBuckets::iterator findBucket(const NodeId &nodeId);

        bool putNode(NodePtr node);

        KBucket::iterator contains(KBuckets::iterator &it, const NodeId &nodeId);

        bool splitBucket(KBuckets::iterator &it);

        std::vector<NodePtr> findNearestNodes(const NodeId &nodeId, std::size_t count);

        NodeId getId();

        NodePtr findNode(const NodeId &nodeId);

        friend std::ostream &operator<<(std::ostream &out, RoutingTable &table);

    private:
        NodeId _id;
        KBuckets _buckets;

    };
}
#endif //P2P_MSG_NODESLIST_H
