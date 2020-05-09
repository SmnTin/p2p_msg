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

        void append(IStreamPtr child) override;

        void setParent(IStreamPtr parent) override;

        //just passes it further
        void performHandshake() override;

        //just passes it further
        void performClosure() override;

        bool opened() const override;

        bool closed() const override;

        void close(IStreamPtr prev) override;
        //true if all the children don't need to be closed
        bool subtreeNeedsToBeClosed() const override;
        void recalcClosureNecessity() override;
        virtual void setClosureNecessity(bool flag);

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
        std::vector<IStreamPtr> _children;

        bool _opened = false;
        bool _closed = false;
        bool _needsToBeClosed = false;
        bool _subtreeNeedsToBeClosed = false;
    };
}

#endif //P2P_MSG_BASIC_STREAM_H
