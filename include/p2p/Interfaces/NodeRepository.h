#ifndef P2P_MSG_NODEREPOSITORY_H
#define P2P_MSG_NODEREPOSITORY_H

#include "p2p/Node.h"
#include <memory>
#include <functional>

//just a usual collection
class INodeRepository {
public:
    virtual NodePtr find(const NodeId &id) = 0;
    virtual NodePtr operator[](const NodeId &id) = 0;

    virtual bool contains(const NodeId &id) = 0;
    virtual size_t count(const NodeId &id) = 0;

    virtual bool empty() = 0;
    virtual std::size_t size() = 0;

    virtual ~NodeRepository() = default;
};
typedef std::shared_ptr<INodeRepository> INodeRepositoryPtr;

#endif //P2P_MSG_NODEREPOSITORY_H
