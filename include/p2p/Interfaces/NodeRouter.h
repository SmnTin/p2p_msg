#ifndef P2P_MSG_NODEROUTER_H
#define P2P_MSG_NODEROUTER_H

#include "p2p/Node.h"
#include <memory>
#include <functional>

class INodeRouter {
public:
    typedef std::function<void(NodePtr)> LocateNodeCallback;

    virtual void locateNode(const NodeId &id, const LocateNodeCallback &callback) = 0;

    virtual ~INodeRouter() = default;
};
typedef std::shared_ptr<INodeRouter> INodeRouterPtr;

#endif //P2P_MSG_PEERROUTER_H
