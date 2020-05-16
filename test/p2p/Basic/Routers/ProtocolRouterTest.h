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

        EXPECT_ANY_THROW(ext0->setChild(ext1));
        EXPECT_ANY_THROW(ext0->setChild(nullptr));

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
        branch->setChild(proto);

        EXPECT_ANY_THROW(branch->setParent(router));
        EXPECT_ANY_THROW(branch->setParent(nullptr));

        //double register
        EXPECT_ANY_THROW(router->registerProtocol(params));
    }

    TEST(ProtocolRouterStream, send_receive) {
        ProtocolParams params1("abc",
                              {1, 0, 0},
                              {0, 9, 4});
        ProtocolParams params2("xyz",
                              {0, 5, 0},
                              {0, 2, 3});

//        auto mock = std::make_shared<testing::NiceMock<StreamMock>>();
        auto mock = std::make_shared<StreamMock>();
        auto router = std::make_shared<ProtocolRouterStream>();
        auto proto1 = std::make_shared<Stream<Messaging::QueuePolicy>>();
        auto proto2 = std::make_shared<Stream<Messaging::QueuePolicy>>();

        router->setParent(mock);
        auto branch1 = router->registerProtocol(params1);
        auto branch2 = router->registerProtocol(params2);
        branch1->setChild(proto1);
        branch2->setChild(proto2);

        router->receive("/abc/0.9.5/hello");
        EXPECT_TRUE(proto1->read() == "hello");
        EXPECT_ANY_THROW(router->receive("/abc/0.9.3/hello"));
        EXPECT_FALSE(proto1->available());

        router->receive("/xyz/1.2.0/hello2");
        EXPECT_TRUE(proto2->read() == "hello2");

        EXPECT_CALL(*mock, send("/abc/1.0.0/hi"));
        proto1->send("hi");
    }
}

#endif //P2P_MSG_PROTOCOLROUTERTEST_H
