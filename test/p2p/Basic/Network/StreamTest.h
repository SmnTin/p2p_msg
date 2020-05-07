#ifndef P2P_MSG_STREAMTEST_H
#define P2P_MSG_STREAMTEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "p2p/Basic/Network/Stream.h"

namespace {
    using namespace p2p::Basic::Network;

    class StreamMock : public IStream {
    public:
        MOCK_METHOD(void, append, (IStreamPtr), (override));
        MOCK_METHOD(void, setParent, (IStreamPtr), (override));
        MOCK_METHOD(void, performHandshake, (), (override));
        MOCK_METHOD(void, performClosure, (), (override));
        MOCK_METHOD(bool, opened, (), (const, override));
        MOCK_METHOD(bool, closed, (), (const, override));
        MOCK_METHOD(void, close, (IStreamPtr), (override));
        MOCK_METHOD(bool, subtreeNeedsToBeClosed, (), (const, override));
        MOCK_METHOD(void, recalcClosureNecessity, (), (override));
        MOCK_METHOD(void, send, (Buffer), (override));
        MOCK_METHOD(void, receive, (Buffer), (override));
    };

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
        auto stream3 = std::make_shared<testing::NiceMock<StreamMock>>();
        stream1->append(stream2);
        stream1->append(stream3);

        Buffer buf = "hello";
        EXPECT_CALL(*stream2, receive(buf))
                .Times(1);
        EXPECT_CALL(*stream3, receive(buf))
                .Times(1);

        stream1->receive(buf);
    }

    TEST(BasicStream, Send) {
        Buffer buf = "hello";

        auto mock = std::make_shared<testing::NiceMock<StreamMock>>();
        EXPECT_CALL(*mock, send(buf))
                .Times(2);

        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();

        stream1->setParent(mock);
        stream1->append(stream2);
        stream1->append(stream3);
        stream3->append(stream4);

        stream2->send(buf);
        stream4->send(buf);
    }

    TEST(BasicStream, PerformHandshake) {
        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<testing::NiceMock<StreamMock>>();
        auto stream5 = std::make_shared<testing::NiceMock<StreamMock>>();

        stream1->append(stream2);
        stream1->append(stream3);
        stream2->append(stream4);
        stream3->append(stream5);

        EXPECT_TRUE(stream1->opened());
        EXPECT_TRUE(stream2->opened());
        EXPECT_TRUE(stream3->opened());

        EXPECT_CALL(*stream4, performHandshake())
                .Times(1);
        EXPECT_CALL(*stream5, performHandshake())
                .Times(1);
        EXPECT_CALL(*stream4, opened())
                .Times(testing::AnyNumber())
                .WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*stream5, opened())
                .Times(testing::AnyNumber())
                .WillRepeatedly(testing::Return(true));

        stream1->performHandshake();
    }

    TEST(BasicStream, performClosure) {
        auto stream1 = std::make_shared<testing::NiceMock<StreamMock>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();
        auto stream5 = std::make_shared<Stream<>>();
        auto stream6 = std::make_shared<Stream<>>();

        stream2->setParent(stream1);
        stream3->setParent(stream1);
        stream2->append(stream4);
        stream3->append(stream5);
        stream3->append(stream6);
        stream6->setClosureNecessity(true);

        EXPECT_CALL(*stream1, performClosure())
                .Times(2);

        EXPECT_FALSE(stream2->closed());
        EXPECT_FALSE(stream3->closed());
        EXPECT_FALSE(stream4->closed());
        EXPECT_FALSE(stream5->closed());
        EXPECT_FALSE(stream6->closed());

        stream4->performClosure();
        EXPECT_TRUE(stream2->closed());
        EXPECT_TRUE(stream4->closed());
        EXPECT_FALSE(stream3->closed());
        EXPECT_FALSE(stream5->closed());
        EXPECT_FALSE(stream6->closed());

        stream5->performClosure();
        EXPECT_FALSE(stream3->closed());
        EXPECT_TRUE(stream5->closed());
        EXPECT_FALSE(stream6->closed());

        stream6->performClosure();
        EXPECT_TRUE(stream3->closed());
        EXPECT_TRUE(stream5->closed());
        EXPECT_TRUE(stream6->closed());
    }

    TEST(BasicStream, needsToBeClosed) {
        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();
        auto stream5 = std::make_shared<Stream<>>();
        auto stream6 = std::make_shared<Stream<>>();

        stream1->append(stream2);
        stream1->append(stream3);
        stream2->append(stream4);
        stream3->append(stream5);
        stream3->append(stream6);
        stream6->setClosureNecessity(true);

        EXPECT_TRUE(stream1->subtreeNeedsToBeClosed());
        EXPECT_FALSE(stream2->subtreeNeedsToBeClosed());
        EXPECT_TRUE(stream3->subtreeNeedsToBeClosed());
        EXPECT_FALSE(stream4->subtreeNeedsToBeClosed());
        EXPECT_FALSE(stream5->subtreeNeedsToBeClosed());
        EXPECT_TRUE(stream6->subtreeNeedsToBeClosed());
    }

    TEST(BasicStream, close) {
        auto stream1 = std::make_shared<Stream<>>();
        auto stream2 = std::make_shared<Stream<>>();
        auto stream3 = std::make_shared<Stream<>>();
        auto stream4 = std::make_shared<Stream<>>();
        auto stream5 = std::make_shared<Stream<>>();
        auto stream6 = std::make_shared<Stream<>>();

        stream1->append(stream2);
        stream1->append(stream3);
        stream2->append(stream4);
        stream3->append(stream5);
        stream3->append(stream6);
        stream4->setClosureNecessity(true);
        stream5->setClosureNecessity(true);
        stream6->setClosureNecessity(true);

        stream2->close(nullptr);

        EXPECT_TRUE(stream1->closed());
        EXPECT_TRUE(stream2->closed());
        EXPECT_TRUE(stream3->closed());
        EXPECT_TRUE(stream4->closed());
        EXPECT_TRUE(stream5->closed());
        EXPECT_TRUE(stream6->closed());
    }
}

#endif //P2P_MSG_STREAMTEST_H
