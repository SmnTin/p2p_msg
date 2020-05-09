#ifndef P2P_MSG_ANYSELECTORTEST_H
#define P2P_MSG_ANYSELECTORTEST_H

#include "p2p/Network/AnySelector.h"

#include "p2p/Basic/Network/ArmedStream.h"

namespace {
    using namespace p2p;
    using namespace p2p::Network;

    TEST(AnySelector, queries) {
        StreamSelector selector;

        NodeId id1 = NodeId::random();
        NodeId id2 = NodeId::random();
        NodeId id3 = NodeId::random();

        Buffer name1 = "abc";
        Buffer name2 = "bcd";
        Buffer name3 = "cde";

        TransportTraits traits1(name1, true, false, true);
        TransportTraits traits2(name2, true, true, false);
        TransportTraits traits3(name3, false, false, true);

        Endpoint endp1, endp2, endp3;

        IStreamPtr stream1 = std::make_shared<ArmedStream>(id1, traits1, endp1);
        IStreamPtr stream2 = std::make_shared<ArmedStream>(id2, traits2, endp2);
        IStreamPtr stream3 = std::make_shared<ArmedStream>(id3, traits3, endp3);

        selector.add(stream1);
        selector.add(stream2);
        selector.add(stream3);

        EXPECT_EQ(selector.anyEnergyInefficient(), stream2);
        EXPECT_EQ(selector.anyFast(), stream2);
        EXPECT_EQ(selector.anyUnreliable(), stream3);

        auto slowStream = selector.anySlow();
        EXPECT_TRUE(slowStream == stream1 || slowStream == stream3);
        slowStream = std::make_shared<Stream<>>();

        auto reliableStream = selector.anyReliable();
        EXPECT_TRUE(reliableStream == stream1 || reliableStream == stream2);
        reliableStream = std::make_shared<Stream<>>();

        auto energyEfficientStream = selector.anyEnergyEfficient();
        EXPECT_TRUE(energyEfficientStream == stream1 || energyEfficientStream == stream3);
        energyEfficientStream = std::make_shared<Stream<>>();

        auto getFastAndReliableStream = [&]() {
            return selector.query
                   < StreamFilter::And
                   < StreamFilter::Fast, StreamFilter::Reliable >> ();
        };

        EXPECT_EQ(getFastAndReliableStream(), stream2);

        auto getFastAndUnreliableStream = [&]() {
            return selector.query
                   < StreamFilter::And
                   < StreamFilter::Fast, StreamFilter::Unreliable >> ();
        };

        EXPECT_ANY_THROW(getFastAndUnreliableStream());

        stream2 = std::make_shared<Stream<>>();

        EXPECT_ANY_THROW(getFastAndReliableStream());
    }
}

#endif //P2P_MSG_ANYSELECTORTEST_H
