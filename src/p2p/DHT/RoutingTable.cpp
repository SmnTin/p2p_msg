#include "p2p/Basic/KadDHT/RoutingTable.h"

namespace p2p::Basic::KadDHT {

    RoutingTable::RoutingTable(std::shared_ptr<HostId> hostId) {
        _id = hostId->getId();
    }

    RoutingTable::~RoutingTable() {

    }

    NodeId RoutingTable::getId() {
        return _id;
    }

    KBuckets::iterator RoutingTable::findBucket(const p2p::NodeId &nodeId) {
        std::size_t lcp = NodeId::longestCommonPrefix(nodeId, _id);

        if(lcp == NodeId::sizeInBits)
            return _buckets.end();

        if(_buckets.empty())
            _buckets.push_back(KBucket());

        std::size_t pos = std::min(lcp, _buckets.size() - 1);

        return std::next(_buckets.begin(), pos);
    }

    //We can split only the last bucket.
    //Nodes with LCP length that equals to (buckets size - 1) remain in the bucket
    //others are moved into a new bucket.
    //Returns true if the bucket was successfully split.
    bool RoutingTable::splitBucket(KBuckets::iterator &it) {
        if(it == _buckets.end() || it != std::prev(_buckets.end()) || _buckets.size() == NodeId::sizeInBits)
            return false;

        KBucket n1, n2;

        for(auto buckIt = it->begin(); buckIt != it->end(); buckIt = std::next(buckIt)) {
            if(NodeId::longestCommonPrefix((*buckIt)->id(), _id) == _buckets.size()-1 )
                n1.push_back(*buckIt);
            else
                n2.push_back(*buckIt);
        }

        if(n2.empty())
            return false;

        *it = n1;
        _buckets.push_back(n2);

        return true;
    }

    KBucket::iterator RoutingTable::contains(KBuckets::iterator & it, const p2p::NodeId &nodeId) {
        auto res = std::find_if(it->begin(), it->end(), [nodeId] (auto & node) -> bool {
            return node->id() == nodeId;
        });

        return res;
    }

    //Returns true if successfully put new node
    bool RoutingTable::putNode(NodePtr node) {
        auto it = findBucket(node->id());

        //node's id equals to ours
        if(it == _buckets.end())
            return false;

        //the node is already in the routing table
        auto containsIt = contains(it, node->id());
        if(containsIt != it->end()) {
//                if((*containsIt)->address() != node->address()) {
//                    node->setExpired();
//                    (*containsIt)->setExpired();
//                }

            //Node might have reconnected to the net and changed ip address
            (*containsIt)->heard(time(0));
            return false;
        }

        //if the bucket is full, we try removing bad nodes
        if(it->size() == MAX_NODES_IN_KBUCKET) {
            for(auto iter = it->begin(); iter != it->end();) {
                if(!(*iter)->isGood(time(0)))
                    (*iter)->setExpired();

                auto next = std::next(iter);

                if((*iter)->isExpired())
                    it->erase(iter);

                iter = next;
            }
        }

        //if the bucket remains full, we try splitting it
        if(it->size() == MAX_NODES_IN_KBUCKET) {
            if(!splitBucket(it))
                return false;

            //this bucket was split in two buckets
            //each with different lcp from ours id
            //so we need to check again which bucket fits best
            it = findBucket(node->id());

            //bad luck
            if (it->size() == MAX_NODES_IN_KBUCKET)
                return false;
        }

        it->push_back(node);

        return true;
    }

    std::vector<NodePtr> RoutingTable::findNearestNodes(const p2p::NodeId &nodeId,
                                                                      std::size_t count) {
        std::vector<std::shared_ptr<Node>> ans;
        auto it = findBucket(nodeId);

        if(it == _buckets.end()) {
//                ans.push_back(std::make_shared<Node>(getId(), _networkEngine->getThisEndpoint()));
            return ans;
        }

        auto sortedBucketInsert = [&](const KBucket &b) {
            for (auto n : b) {
                if (!n->isGood(time(0)))
                    continue;

                auto here = std::find_if(ans.begin(), ans.end(),
                                         [&nodeId,&n](std::shared_ptr<Node> &node) {
                                             return nodeId.xorCmp(n->id(), node->id()) < 0;
                                         }
                );
                ans.insert(here, n);
            }
        };

        sortedBucketInsert(*it);

        if(ans.size() < count) {
            auto itn = it, itp = it;

            do {
                itn = std::next(itn);
                itp = (itp == _buckets.begin() ? _buckets.end() : std::prev(itp));

                if(itn != _buckets.end())
                    sortedBucketInsert(*itn);

                if(itp != _buckets.end())
                    sortedBucketInsert(*itp);

            } while(itn != _buckets.end() || itp != _buckets.end());
        }

        if(ans.size() > count)
            ans.resize(count);

        return ans;
    }

    NodePtr RoutingTable::findNode(const NodeId &nodeId) {
        auto it = findBucket(nodeId);

        //node's id equals to ours
        if(it == _buckets.end())
            return nullptr;

        auto containsIt = contains(it, nodeId);
        //the node isn't in the routing table
        if(containsIt == it->end())
            return nullptr;

        return *containsIt;
    }

    std::ostream & operator<< (std::ostream & out, RoutingTable & table) {
        out << "myId\n";
        out << table.getId() << "\n\n\n";
        KBuckets & buckets = table._buckets;
        out << "There are " << buckets.size() << " buckets\n";
        for(std::size_t i = 0; i < buckets.size(); ++i) {
            out << "Bucket " << i << ":\n";
            for(auto it = buckets[i].begin(); it != buckets[i].end(); it = std::next(it)) {
                out << (*it)->id() << "\n";
                out << ((*it)->isGood(time(0)) ? "Good" : "Bad") << "\n";
                out << "\n";
            }
            out << "\n\n\n";
        }

        return out;
    }

} //KadDHT