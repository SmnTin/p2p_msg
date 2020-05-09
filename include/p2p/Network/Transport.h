#ifndef P2P_MSG_NETWORK_TRANSPORT_H
#define P2P_MSG_NETWORK_TRANSPORT_H

#include "p2p/Network/Endpoint.h"
#include "p2p/NodeId.h"

#include <memory>

namespace p2p::Network {
    class ITransport {
    public:
        virtual Buffer getName() const = 0;
        virtual TransportTraits getTraits() const = 0;

        virtual void connect(const NodeId &nodeId, const Endpoint &endpoint) = 0;
    };

    typedef std::shared_ptr<ITransport> ITransportPtr;
    typedef std::weak_ptr<ITransport> ITransportWPtr;
}

#endif //P2P_MSG_NETWORK_TRANSPORT_H
