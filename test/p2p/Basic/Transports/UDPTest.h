#ifndef P2P_MSG_TEST_TRANSPORTS_UDP_H
#define P2P_MSG_TEST_TRANSPORTS_UDP_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "p2p/Basic/Transports/UDP.h"
#include "p2p/Basic/Transports/details/UDP/EndpointTranslation.h"
#include "p2p/Basic/Host/HostKeyPair.h"
#include "p2p/Basic/Network/ExtensionWithStreamStore.h"
#include "p2p/Basic/StreamEncryption/ECC.h"
#include "p2p/Basic/StreamProtection/Nonce.h"

namespace {
    using namespace p2p::Basic::Network;
    using namespace p2p::Basic;

    TEST(UDPTransport, EndpointTranslationTest) {
        UDPSocketEndpoint sockEndp1(
                boost::asio::ip::address_v4::from_string("192.172.105.5"),
                18090);
        auto endpoint1 = UDPEndpointTranslation::serialize(sockEndp1);
        auto sockEndp2 = UDPEndpointTranslation::deserialize(endpoint1);
        EXPECT_EQ(sockEndp1, sockEndp2);

        UDPSocketEndpoint sockEndp3(
                boost::asio::ip::address_v6::from_string(
                        "EF01:162B:AB09:CD45:EFFF:165A:ABBA:CDE4"),
                15050);
        auto endpoint2 = UDPEndpointTranslation::serialize(sockEndp3);
        auto sockEndp4 = UDPEndpointTranslation::deserialize(endpoint2);
        EXPECT_EQ(sockEndp3, sockEndp4);
    }

    //ALL IN ONE!!!
    TEST(UDPTransport, TwoEndpointsConnectTest) {
        auto context = std::make_shared<boost::asio::io_context>();
        auto hostId1 = std::make_shared<HostKeyPair>();
        auto hostId2 = std::make_shared<HostKeyPair>();

        UDPTransport transport1(context, hostId1, 4109, true);
        UDPTransport transport2(context, hostId2, 4110, true);

        auto ecc1 = std::make_shared<ECCExtension>(hostId1);
        auto ecc2 = std::make_shared<ECCExtension>(hostId2);
        auto nonce1 = std::make_shared<ExtensionWithNonce<uint32_t, 20>>();
        auto nonce2 = std::make_shared<ExtensionWithNonce<uint32_t, 20>>();
        auto superExtension1 = std::make_shared<ExtensionWithStreamStore>();
        auto superExtension2 = std::make_shared<ExtensionWithStreamStore>();
        transport1.extend(nonce1);
        transport2.extend(nonce2);
        nonce1->append(ecc1);
        nonce2->append(ecc2);
        ecc1->append(superExtension1);
        ecc2->append(superExtension2);

        Subscription handle;
        bool received = false;
        transport1.connect(hostId2->getId(), transport2.getEndpoint())
                .then([&]() {
                    //Generally, connection opening should be one way
                    // but... there is no easy way to know when the message is received by the
                    // UDP socket so let's open the connection twice
                    // At any rate, no handshake messages would be sent with this nodes setup
                    return transport2.connect(hostId1->getId(), transport1.getEndpoint());
                })
                .then([&]() {
                    auto stream1 = superExtension1->getStore()[hostId2->getId()].any();
                    auto stream2 = superExtension2->getStore()[hostId1->getId()].any();
                    auto receiver = std::make_shared<Stream<Messaging::SubscriptionPolicy>>();
                    stream2->append(receiver);
                    Buffer sentMsg = "FUCK!!!\n";
                    auto callback = [&, sentMsg](Buffer receivedMsg) {
                        EXPECT_EQ(sentMsg, receivedMsg);
                        received = true;
                        context->stop();
                    };
                    handle = receiver->subscribe(callback);
                    stream1->send(sentMsg);
                });
        context->run_for(std::chrono::milliseconds{100});
        EXPECT_TRUE(received);
    }
}

#endif //P2P_MSG_TEST_TRANSPORTS_UDP_H
