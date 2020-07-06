#ifndef P2P_MSG_TRANSPORTTRAITS_H
#define P2P_MSG_TRANSPORTTRAITS_H

#include "p2p/Network/Transport.h"

namespace p2p::Network {
    static const TransportTraits UDPTransportTraits {
            "UDP", false, true, false
    };
}

#endif //P2P_MSG_TRANSPORTTRAITS_H
