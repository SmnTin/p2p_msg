#ifndef P2P_MSG_NETWORK_TRANSPORT_H
#define P2P_MSG_NETWORK_TRANSPORT_H

#include "p2p/Network/Endpoint.h"
#include "p2p/Network/Extension.h"
#include "p2p/NodeId.h"

#include "continuable/continuable.hpp"

#include <memory>

namespace p2p::Network {
    class ITransport {
    public:
        virtual TransportTraits getTraits() const = 0;

        //this method must not acquire ownership over extension by append(...) method
        //it must just store the pointer and pass new streams to it on creation
        virtual void extend(IExtensionPtr extension) = 0;

        virtual cti::continuable<>
        connect(const NodeId &nodeId, const Endpoint &endpoint) = 0;
    };

    typedef std::shared_ptr<ITransport> ITransportPtr;
    typedef std::weak_ptr<ITransport> ITransportWPtr;
}

#endif //P2P_MSG_NETWORK_TRANSPORT_H
