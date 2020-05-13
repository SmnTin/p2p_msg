#ifndef P2P_MSG_BASIC_STREAM_H
#define P2P_MSG_BASIC_STREAM_H

#include "p2p/Network/Stream.h"
#include <queue>
#include <vector>

namespace p2p::Basic::Network {
    using namespace p2p::Network;

    namespace Messaging {
        class IPolicy {
        public:
            virtual void spreadMessage(Buffer message) = 0;
        };

        class NoPolicy : public IPolicy {
        protected:
            void spreadMessage(Buffer message) override;
        };

        class SubscriptionPolicy : public IPolicy {
        public:
            typedef std::function<void(Buffer)> MessageCallback;

            [[nodiscard]]
            virtual Subscription subscribe(MessageCallback callback);

        protected:
            void spreadMessage(Buffer message) override;

            Publisher<Buffer> _publisher;
        };

        class QueuePolicy : public IPolicy {
        public:
            virtual bool available() const;

            virtual Buffer read();

        protected:
            void spreadMessage(Buffer message) override;

            std::queue<Buffer> _msgQueue;
        };

    }

    template<class MessagingPolicy = Messaging::NoPolicy>
    class Stream : public IStream, public MessagingPolicy {
    public:
        Stream();

        void setChild(IStreamPtr child) override;
        void setChild(std::nullptr_t child) override;

        void setParent(IStreamPtr parent) override;
        void setParent(std::nullptr_t parent) override;

        //just passes it further
        void performHandshake() override;

        //just passes it further
        void performClosure() override;

        bool opened() const override;

        bool closed() const override;

        void close(IStreamPtr prev) override;

        void send(Buffer msg) override;
        void receive(Buffer msg) override;

        NodeId getNodeId() const override;
        NodeId getNodeId() override;

        TransportTraits getTraits() const override;
        TransportTraits getTraits() override;

        Endpoint getEndpoint() const override;
        Endpoint getEndpoint() override;

    protected:
        std::optional<NodeId> _nodeId;
        std::optional<TransportTraits> _transportTraits;
        std::optional<Endpoint> _endpoint;

        IStreamWPtr _parent;
        //holds strong pointer to the child to manage its memory
        IStreamPtr _child;

        bool _opened = false;
        bool _closed = false;
    };
}

#endif //P2P_MSG_BASIC_STREAM_H
