#ifndef P2P_MSG_STREAMTEST_H
#define P2P_MSG_STREAMTEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "p2p/Basic/Network/Stream.h"

namespace {
    using namespace p2p::Basic::Network;

    class MockStream : public Stream {
    public:
        MOCK_METHOD(void, append, (IStreamPtr
                child), (override));
        MOCK_METHOD(void, setParent, (IStreamPtr
                parent), (override));
        MOCK_METHOD(void, performHandshake, (), (override));
        MOCK_METHOD(void, performClosure, (), (override));
        MOCK_METHOD(bool, opened, (), (const, override));
        MOCK_METHOD(bool, closed, (), (const, override));
        MOCK_METHOD(bool, needsToBeClosed, (), (const, override));
        MOCK_METHOD(Subscription, subscribe, (MessageCallback
                callback), (override));
        MOCK_METHOD(void, send, (Buffer
                msg), (override));
        MOCK_METHOD(void, receive, (Buffer
                msg), (override));
    };

    TEST(BasicStream, Creation) {
        EXPECT_NO_THROW(std::make_shared<Stream>());
    }

    TEST(BasicStream, Subscription) {
        auto stream = std::make_shared<Stream>();

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

    TEST(BasicStream, Receive) {
        auto stream1 = std::make_shared<Stream>();
        auto stream2 = std::make_shared<testing::NiceMock<MockStream>>();
        auto stream3 = std::make_shared<testing::NiceMock<MockStream>>();
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

        auto mock = std::make_shared<testing::NiceMock<MockStream>>();
        EXPECT_CALL(*mock, send(buf))
                .Times(2);

        auto stream1 = std::make_shared<Stream>();
        auto stream2 = std::make_shared<Stream>();
        auto stream3 = std::make_shared<Stream>();
        auto stream4 = std::make_shared<Stream>();

        stream1->setParent(mock);
        stream1->append(stream2);
        stream1->append(stream3);
        stream3->append(stream4);

        stream2->send(buf);
        stream4->send(buf);
    }
}

#endif //P2P_MSG_STREAMTEST_H
