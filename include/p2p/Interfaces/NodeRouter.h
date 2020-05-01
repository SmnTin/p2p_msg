#ifndef P2P_MSG_NODEROUTER_H
#define P2P_MSG_NODEROUTER_H

#include "p2p/Node.h"
#include <memory>
#include <functional>

class NodeRouter {
public:
    typedef std::function<void(NodePtr)> LocateNodeCallback;

    virtual void locateNode(const NodeId &id, const LocateNodeCallback &callback) = 0;

    virtual ~NodeRouter() = default;
};

#endif //P2P_MSG_PEERROUTER_H
