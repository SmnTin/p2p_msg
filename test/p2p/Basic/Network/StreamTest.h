#ifndef P2P_MSG_STREAMTEST_H
#define P2P_MSG_STREAMTEST_H

#include "p2p/Basic/Network/StreamMock.h"
#include "p2p/Basic/Network/ArmedStream.h"

namespace {
    using namespace p2p::Basic::Network;

    TEST(BasicStream, Creation) {
        EXPECT_NO_THROW(std::make_shared<Stream<Messaging::NoPolicy>>());
        EXPECT_NO_THROW(std::make_shared<Stream<Messaging::SubscriptionPolicy>>());
        EXPECT_NO_THROW(std::make_shared<Stream<Messaging::QueuePolicy>>());
    }

    TEST(BasicStream, SubscriptionPolicy) {
        auto stream = std::make_shared<Stream<Messaging::SubscriptionPolicy>>();

        int invocationCnt = 0;
        Buffer recv;
        auto cb = [&](Buffer msg) {
            recv = std::move(msg);
            invocationCnt++;
        };

        auto sub = stream->subscribe(cb);
        EXPECT_TRUE(sub.active());
        EXPECT_EQ(invocationCnt, 0);

        Buffer msg1 = "hello";
        stream->receive(msg1);

        EXPECT_EQ(invocationCnt, 1);
        EXPECT_EQ(recv, msg1);

        EXPECT_TRUE(sub.active());
        sub.unsubscribe();
        EXPECT_FALSE(sub.active());

        Buffer msg2 = "hi!";
        stream->receive(msg2);

        EXPECT_EQ(invocationCnt, 1);
        EXPECT_EQ(recv, msg1);
        EXPECT_NE(recv, msg2);
    }

    TEST(BasicStream, QueuePolicy) {
        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<Messaging::QueuePolicy>>();

        EXPECT_FALSE(stream2->available());
        stream1->append(stream2);

        EXPECT_FALSE(stream2->available());
        Buffer msg1 = "goga";
        Buffer msg2 = "guga";
        stream1->receive(msg1);
        stream1->receive(msg2);
        EXPECT_TRUE(stream2->available());

        Buffer recv1 = stream2->read();
        EXPECT_EQ(recv1, msg1);
        EXPECT_TRUE(stream2->available());

        Buffer recv2 = stream2->read();
        EXPECT_EQ(recv2, msg2);
        EXPECT_FALSE(stream2->available());

        EXPECT_ANY_THROW(stream2->read());
    }

    TEST(BasicStream, Receive) {
        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<testing::NiceMock<StreamMock>>();
        stream1->append(stream2);

        Buffer buf = "hello";
        EXPECT_CALL(*stream2, receive(buf))
                .Times(1);

        stream1->receive(buf);
    }

    TEST(BasicStream, Send) {
        Buffer buf = "hello";

        auto mock = std::make_shared<testing::NiceMock<StreamMock>>();
        EXPECT_CALL(*mock, send(buf))
                .Times(1);

        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();

        stream1->setParent(mock);
        stream1->append(stream2);
        stream2->append(stream3);

        stream2->send(buf);
    }

    TEST(BasicStream, PerformHandshake) {
        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<testing::NiceMock<StreamMock>>();

        stream1->append(stream2);
        stream2->append(stream3);
        stream3->append(stream4);

        EXPECT_TRUE(stream1->opened());
        EXPECT_TRUE(stream2->opened());
        EXPECT_TRUE(stream3->opened());

        EXPECT_CALL(*stream4, performHandshake())
                .Times(1);
        EXPECT_CALL(*stream4, opened())
                .Times(testing::AnyNumber())
                .WillRepeatedly(testing::Return(true));

        stream1->performHandshake();
    }

    TEST(BasicStream, performClosure) {
        auto stream1 = std::make_shared<testing::NiceMock<StreamMock>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();

        stream2->setParent(stream1);
        stream2->append(stream3);
        stream3->append(stream4);

        EXPECT_CALL(*stream1, performClosure())
                .Times(1);

        EXPECT_FALSE(stream2->closed());
        EXPECT_FALSE(stream3->closed());
        EXPECT_FALSE(stream4->closed());

        stream4->performClosure();

        EXPECT_TRUE(stream2->closed());
        EXPECT_TRUE(stream3->closed());
        EXPECT_TRUE(stream4->closed());
    }

    TEST(BasicStream, close) {
        auto stream1 = std::make_shared<testing::NiceMock<StreamMock>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();

        stream2->setParent(stream1);
        stream2->append(stream3);
        stream3->append(stream4);

        EXPECT_CALL(*stream1, performClosure())
                .Times(1);

        EXPECT_FALSE(stream2->closed());
        EXPECT_FALSE(stream3->closed());
        EXPECT_FALSE(stream4->closed());

        stream2->close(nullptr);

        EXPECT_TRUE(stream2->closed());
        EXPECT_TRUE(stream3->closed());
        EXPECT_TRUE(stream4->closed());
    }

    TEST(BasicStream, getNodeId_getEndpoint_getTraits) {
        NodeId id = NodeId::random();
        TransportTraits traits("abc", true, false, true);
        Endpoint endp{"abc", "bcd"};
        auto stream1 = std::make_shared<ArmedStream>(id, traits, endp);
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();
        auto stream5 = std::make_shared<Stream<>>();
        auto stream6 = std::make_shared<Stream<>>();

        stream1->append(stream2);
        stream2->append(stream3);
        stream3->append(stream4);
        stream4->append(stream5);
        stream5->append(stream6);

        EXPECT_EQ(stream3->getNodeId(), id);
        EXPECT_EQ(stream4->getNodeId(), id);
        EXPECT_EQ(stream5->getNodeId(), id);
        EXPECT_EQ(stream6->getNodeId(), id);
        EXPECT_EQ(stream2->getNodeId(), id);
        EXPECT_EQ(stream1->getNodeId(), id);

        EXPECT_EQ(stream3->getTraits(), traits);
        EXPECT_EQ(stream4->getTraits(), traits);
        EXPECT_EQ(stream5->getTraits(), traits);
        EXPECT_EQ(stream6->getTraits(), traits);
        EXPECT_EQ(stream2->getTraits(), traits);
        EXPECT_EQ(stream1->getTraits(), traits);

        EXPECT_EQ(stream3->getEndpoint(), endp);
        EXPECT_EQ(stream4->getEndpoint(), endp);
        EXPECT_EQ(stream5->getEndpoint(), endp);
        EXPECT_EQ(stream6->getEndpoint(), endp);
        EXPECT_EQ(stream2->getEndpoint(), endp);
        EXPECT_EQ(stream1->getEndpoint(), endp);
    }

    TEST(BasicStream, AutoSetParent) {
        auto mock = std::make_shared<testing::NiceMock<StreamMock>>();
        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();

        stream1->setParent(mock);

        EXPECT_CALL(*mock, send("a"))
                .Times(1);
        EXPECT_CALL(*mock, send("b"))
                .Times(1);

        stream1->append(stream2);
        stream2->send("a");

        stream1->append(stream3);
        stream2->send("a");
        stream3->send("b");
    }

    TEST(BasicStream, reportThatOpened) {
        auto stream1 = std::make_shared<testing::NiceMock<StreamMock>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();

        stream2->setParent(stream1);
        stream2->append(stream3);
        stream3->append(stream4);

        EXPECT_CALL(*stream1, reportThatOpened())
                .Times(1);

        stream2->performHandshake();
    }
}

#endif //P2P_MSG_STREAMTEST_H
