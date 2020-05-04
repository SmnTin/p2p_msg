#ifndef P2P_MSG_NETWORK_TRANSPORT_H
#define P2P_MSG_NETWORK_TRANSPORT_H

namespace p2p::Network {
    struct Endpoint {
        Buffer transportName;
        Buffer data;
    };

    class IConnection {
        virtual Buffer getTransportName() = 0;

        virtual Endpoint getEndpoint() = 0;

        virtual void send(const Buffer &msg) = 0;

        //Unless transport supports fast but unreliable way of msg delivery (for example, UDP), use send(msg)
        virtual void sendAnyway(const Buffer &msg) = 0;

        virtual void disconnect() = 0;

        virtual bool connected() = 0;

        virtual ~IConnection() {
            if (connected())
                disconnect();
        }
    };
    typedef std::shared_ptr<IConnection> IConnectionPtr;

    class ITransport {
    public:
        virtual Buffer getName() = 0;

        virtual IConnectionPtr connect(const NodeId &nodeId, const Endpoint &endpoint) = 0;
    };
}

#endif //P2P_MSG_NETWORK_TRANSPORT_H
