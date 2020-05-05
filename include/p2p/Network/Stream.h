#ifndef P2P_MSG_STREAM_H
#define P2P_MSG_STREAM_H

#include <memory>

#include "p2p/Buffer.h"
#include "p2p/Common/Subscription.h"

namespace p2p::Network {
    class IStream;

    //use only these classes to own instances of IStream
    typedef std::shared_ptr<IStream> IStreamPtr;
    typedef std::weak_ptr<IStream> IStreamWPtr;

    class IStream : public std::enable_shared_from_this<IStream> {
    public:
        virtual void append(IStreamPtr child) = 0;

        //please invoke it only inside IStream derived classes
        virtual void setParent(IStreamPtr parent) = 0;

        //this method is invoked when the connection has been opened by this side
        virtual void performHandshake() = 0;

        virtual void performClosure() = 0;

        //true if handshake was successful regardless the invocation side
        virtual bool opened() const = 0;

        //true if closure was successful regardless the invocation side
        virtual bool closed() const = 0;
        virtual bool needsToBeClosed() const = 0;
        virtual void setClosureNecessity(bool flag) = 0;

        virtual void receive(Buffer buf) = 0;
        virtual void send(Buffer buf) = 0;

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
