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

    //use only these classes to manage instances of IStream
    //recommended ptr to explicitly own the intance
    typedef std::shared_ptr<IStream> IStreamPtr;
    //recommended ptr to use inside the regular code
    typedef std::weak_ptr<IStream> IStreamWPtr;

    //basically just a linked list
    // but can be split by routing IStream nodes
    class IStream : public inheritable_enable_shared_from_this<IStream> {
    public:
        //may throw if concrete IStream provides another appending mechanism
        //should automatically invoke setParent() of child
        virtual void append(IStreamPtr child) = 0;

        //may throw if concrete IStream provides another appending mechanism
        virtual void setChild(IStreamPtr child) = 0;

        //should be automatically invoked inside append() of parent
        // or other appending mechanism to create bidirectional link
        virtual void setParent(IStreamPtr parent) = 0;

        //this method is invoked when the connection has been opened by this side
        //invocation flow goes from parent to children
        virtual void performHandshake() = 0;

        //this method should be invoked by children to notify that they and their children were opened successfully
        //invocation flow goes from children to parent
        virtual void reportThatOpened() = 0;

        //this method is invoked when the connection is going to be closed by this side
        //invocation flow goes from children to parent
        virtual void performClosure() = 0;

        //true if handshake was successful regardless the invocation side
        virtual bool opened() = 0;

        //should be implemented as simple dfs traverse
        //can be initiated from any node
        //will close the whole tree by calling performClosure of the leaf nodes
        virtual void close(IStreamPtr prev) = 0;

        //true if closure was successful regardless the invocation side
        virtual bool closed() = 0;

        //the message was received by transport and should be propogated to the children
        //invocation flow goes from parent to children
        virtual void receive(Buffer buf) = 0;

        //invocation flow goes from children to parent
        virtual void send(Buffer buf) = 0;

        //must be set once at tree creation
        // and lazily propagated through the tree
        virtual NodeId getNodeId() = 0;

        virtual TransportTraits getTraits() = 0;

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
