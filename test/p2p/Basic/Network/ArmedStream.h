#ifndef P2P_MSG_ARMEDSTREAM_H
#define P2P_MSG_ARMEDSTREAM_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "p2p/Basic/Network/Stream.h"

namespace {
    using namespace p2p::Basic::Network;

    class ArmedStream : public Stream<> {
    public:
        explicit ArmedStream(NodeId id, TransportTraits traits, Endpoint endpoint) {
            _nodeId = id;
            _transportTraits = std::move(traits);
            _endpoint = std::move(endpoint);
        }
    };
}


#endif //P2P_MSG_ARMEDSTREAM_H
