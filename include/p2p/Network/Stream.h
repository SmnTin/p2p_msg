#ifndef P2P_MSG_STREAM_H
#define P2P_MSG_STREAM_H

#include <memory>

#include "p2p/Buffer.h"
#include "p2p/Common/Subscription.h"
#include "p2p/NodeId.h"
#include "p2p/Network/Endpoint.h"

namespace p2p::Network {
    class IStream;

    //use only these classes to own instances of IStream
    typedef std::shared_ptr<IStream> IStreamPtr;
    typedef std::weak_ptr<IStream> IStreamWPtr;

    class IStream : public std::enable_shared_from_this<IStream> {
    public:
        virtual void append(IStreamPtr child) = 0;

        //should be automatically invoked inside append() to create bidirectional link
        virtual void setParent(IStreamPtr parent) = 0;

        //this method is invoked when the connection has been opened by this side
        virtual void performHandshake() = 0;

        //this method is invoked when the connection is going to be closed by this side
        virtual void performClosure() = 0;

        //true if handshake was successful regardless the invocation side
        virtual bool opened() const = 0;

        //should be implemented as simple dfs traverse
        //can be initiated from any node
        //will close the whole tree
        virtual void close(IStreamPtr prev) = 0;

        //true if closure was successful regardless the invocation side
        virtual bool closed() const = 0;

        //return true if any stream in the subtree needs to be closed
        //stream whose field is true won't be destroyed until close() is true too
        virtual bool subtreeNeedsToBeClosed() const = 0;

        //goes up to the root
        virtual void recalcClosureNecessity() = 0;

        virtual void receive(Buffer buf) = 0;
        virtual void send(Buffer buf) = 0;

        //must be set once at tree creation
        virtual NodeId getNodeId() const = 0;
        //and lazily propagated through the tree
        virtual NodeId getNodeId() = 0;

        virtual TransportTraits getTraits() const = 0;
        virtual TransportTraits getTraits() = 0;

        virtual Endpoint getEndpoint() const = 0;
        virtual Endpoint getEndpoint() = 0;

        virtual ~IStream() = default;

        friend IStreamPtr &operator<<(IStreamPtr &out, const Buffer &buf) {
            out->send(buf);
            return out;
        };
//        friend void operator>>(const Buffer &buf, IStreamPtr &in) {
//            in->receive(buf);
//        }
    };
}

#endif //P2P_MSG_STREAM_H
