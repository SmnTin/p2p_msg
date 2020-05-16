#ifndef P2P_MSG_STREAM_H
#define P2P_MSG_STREAM_H

#include <memory>

#include "p2p/Buffer.h"
#include "p2p/Common/Subscription.h"
#include "p2p/NodeId.h"
#include "p2p/Network/Endpoint.h"
#include "p2p/Common/InheritableSharedFromThis.h"

namespace p2p::Network {
    class IStream;

    //use only these classes to own instances of IStream
    typedef std::shared_ptr<IStream> IStreamPtr;
    typedef std::weak_ptr<IStream> IStreamWPtr; //recommended ptr to use inside the code

    //basically just a linked list
    //but can be split by routing IStream nodes
    class IStream : public inheritable_enable_shared_from_this<IStream> {
    public:
        //may throw if concrete IStream provides another appending mechanism
        virtual void setChild(IStreamPtr child) = 0;
        virtual void setChild(std::nullptr_t child) = 0;

        //should be automatically invoked inside setChild()
        // or other appending mechanism to create bidirectional link
        virtual void setParent(IStreamPtr parent) = 0;
        virtual void setParent(std::nullptr_t parent) = 0;

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
