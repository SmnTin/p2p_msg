#ifndef P2P_MSG_STREAMMOCK_H
#define P2P_MSG_STREAMMOCK_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "p2p/Basic/Network/Stream.h"

namespace {
    using namespace p2p;
    using namespace p2p::Basic::Network;

    class StreamMock : public IStream {
    public:
        MOCK_METHOD(void, setChild, (IStreamPtr), (override));
        MOCK_METHOD(void, setChild, (std::nullptr_t), (override));
        MOCK_METHOD(void, setParent, (IStreamPtr), (override));
        MOCK_METHOD(void, setParent, (std::nullptr_t), (override));
        MOCK_METHOD(void, performHandshake, (), (override));
        MOCK_METHOD(void, performClosure, (), (override));
        MOCK_METHOD(bool, opened, (), (const, override));
        MOCK_METHOD(bool, closed, (), (const, override));
        MOCK_METHOD(void, close, (IStreamPtr), (override));
        MOCK_METHOD(void, send, (Buffer), (override));
        MOCK_METHOD(void, receive, (Buffer), (override));
        MOCK_METHOD(NodeId, getNodeId, (), (override));
        MOCK_METHOD(NodeId, getNodeId, (), (const, override));
        MOCK_METHOD(Endpoint, getEndpoint, (), (override));
        MOCK_METHOD(Endpoint, getEndpoint, (), (const, override));
        MOCK_METHOD(TransportTraits, getTraits, (), (override));
        MOCK_METHOD(TransportTraits, getTraits, (), (const, override));
    };
}

#endif //P2P_MSG_STREAMMOCK_H
