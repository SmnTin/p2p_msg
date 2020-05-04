#ifndef P2P_MSG_BASIC_STREAM_H
#define P2P_MSG_BASIC_STREAM_H

#include "p2p/Network/Stream.h"

namespace p2p::Basic::Network {
    using namespace p2p::Network;

    class Stream : public IStream {
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

        bool needsToBeClosed() const override;

        Subscription subscribe(MessageCallback callback) override;

        void send(Buffer msg) override;
        void receive(Buffer msg) override;

    protected:

        IStreamWPtr _parent;
        std::vector<IStreamPtr> _children;

        Publisher<Buffer> _publisher;

        bool _opened = false,
             _closed = false;
    };
}

#endif //P2P_MSG_BASIC_STREAM_H
