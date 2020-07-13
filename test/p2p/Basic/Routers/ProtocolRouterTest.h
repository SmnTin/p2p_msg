#ifndef P2P_MSG_PROTOCOLROUTERTEST_H
#define P2P_MSG_PROTOCOLROUTERTEST_H

#include "p2p/Basic/Network/StreamMock.h"
#include "p2p/Basic/Network/ArmedStream.h"
#include "p2p/Basic/Routers/ProtocolRouter.h"

#include "p2p/Common/di.h"

namespace {
    using namespace p2p::Basic::Network;

    TEST(ProtocolRouter, Creation) {
        //unfortunately you cannot pass string literal as the template parameter
        //so it's better to include name in class as a static string literal field
        static const char firstName[] = "ab";
        static const char secondName[] = "bc";
        auto inj1 = di::make_injector();
        IExtensionPtr ext = inj1.create<
                Builder::ProtocolRouter<
                        Builder::Protocol<
                                Builder::Name<firstName>,
                                Builder::Version<1, 0, 0>,
                                Builder::Version<0, 0, 1>,
                                Extension
                        >,
                        Builder::Protocol<
                                Builder::Name<secondName>,
                                Builder::Version<1, 0, 0>,
                                Builder::Version<0, 0, 1>,
                                Extension
                        >
                >
        >();

        auto stream = std::make_shared<Stream<>>();
        ext->extendStream(stream);
    }

    TEST(ProtocolRouter, setChild) {
        auto ext0 = std::make_shared<ProtocolRouterExtension>();
        auto ext1 = std::make_shared<Extension>();

        EXPECT_ANY_THROW(ext0->append(ext1));
        EXPECT_ANY_THROW(ext0->append(nullptr));

        auto stream0 = std::make_shared<ProtocolRouterStream>();
        auto stream1 = std::make_shared<ProtocolRouterStream>();

        EXPECT_ANY_THROW(stream0->setChild(stream1));
        EXPECT_ANY_THROW(stream0->setChild(nullptr));
    }

    TEST(ProtocolParams, Constructon) {
        EXPECT_ANY_THROW(ProtocolParams("abc$", {1, 0, 0}, {0, 9, 9}));
        EXPECT_NO_THROW(ProtocolParams("abz1239._-", {1, 0, 0}, {0, 9, 9}));
        EXPECT_ANY_THROW(ProtocolParams("abz1239._-", {1, 0, 0}, {1, 0, 1}));
    }

    TEST(ProtocolRouterStream, registerProtocol) {
        ProtocolParams params("abc",
                              {1, 0, 0},
                              {0, 9, 9});

        auto router = std::make_shared<ProtocolRouterStream>();
        auto proto = std::make_shared<Stream<>>();

        auto branch = router->registerProtocol(params);
        branch->append(proto);

        EXPECT_ANY_THROW(branch->setParent(router));
        EXPECT_ANY_THROW(branch->setParent(nullptr));

        //double register
        EXPECT_ANY_THROW(router->registerProtocol(params));
    }

    class ProtocolRouterStreamTestFixture : public testing::Test {
    protected:
        void SetUp() override {
            params1 = ProtocolParams("abc",
                                   {1, 0, 0},
                                   {0, 9, 4});
            params2 = ProtocolParams("xyz",
                                   {0, 5, 0},
                                   {0, 2, 3});
            mock = std::make_shared<testing::NiceMock<StreamMock>>();
            router = std::make_shared<ProtocolRouterStream>();

            router->setParent(mock);

            branch1 = router->registerProtocol(params1);
            branch2 = router->registerProtocol(params2);
        }

        ProtocolParams params1, params2;
        std::shared_ptr<testing::NiceMock<StreamMock>> mock;
        std::shared_ptr<ProtocolRouterStream> router;
        IStreamPtr branch1, branch2;
    };

    TEST_F(ProtocolRouterStreamTestFixture, send_receive) {
        auto proto1 = std::make_shared<Stream<Messaging::QueuePolicy>>();
        auto proto2 = std::make_shared<Stream<Messaging::QueuePolicy>>();

        branch1->append(proto1);
        branch2->append(proto2);

        router->receive("/abc/0.9.5/hello");
        EXPECT_TRUE(proto1->read() == "hello");
        EXPECT_ANY_THROW(router->receive("/abc/0.9.3/hello"));
        EXPECT_FALSE(proto1->available());

        router->receive("/xyz/1.2.0/hello2");
        EXPECT_TRUE(proto2->read() == "hello2");

        EXPECT_CALL(*mock, send("/abc/1.0.0/hi"));
        proto1->send("hi");
    }

    TEST_F(ProtocolRouterStreamTestFixture, performHandshake) {
        auto proto1 = std::make_shared<testing::NiceMock<StreamMock>>();
        auto proto2 = std::make_shared<testing::NiceMock<StreamMock>>();

        branch1->append(proto1);
        branch2->append(proto2);

        EXPECT_CALL(*mock, reportThatOpened())
            .Times(1);
        EXPECT_CALL(*proto1, performHandshake())
                .Times(1);
        EXPECT_CALL(*proto2, performHandshake())
                .Times(1);

        EXPECT_TRUE(router->opened());
        EXPECT_TRUE(branch1->opened());
        EXPECT_TRUE(branch2->opened());

        router->performHandshake();

        branch1->reportThatOpened();
        branch2->reportThatOpened();
    }

    TEST_F(ProtocolRouterStreamTestFixture, performClosure) {
        auto proto1 = std::make_shared<Stream<>>();
        auto proto2 = std::make_shared<Stream<>>();

        branch1->append(proto1);
        branch2->append(proto2);

        EXPECT_CALL(*mock, performClosure())
                .Times(1);

        EXPECT_FALSE(router->closed());
        EXPECT_FALSE(branch1->closed());
        EXPECT_FALSE(branch2->closed());

        router->close(nullptr);

        EXPECT_TRUE(router->closed());
        EXPECT_TRUE(branch1->closed());
        EXPECT_TRUE(branch2->closed());
    }
}

#endif //P2P_MSG_PROTOCOLROUTERTEST_H
