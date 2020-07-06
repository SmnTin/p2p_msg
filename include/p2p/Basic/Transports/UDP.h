#ifndef P2P_MSG_TRANSPORTS_UDP_H
#define P2P_MSG_TRANSPORTS_UDP_H

#include "p2p/Network/Transport.h"

#include "p2p/Host/HostId.h"

#include "boost/asio.hpp"

namespace p2p::Basic::Network {
    using namespace p2p::Network;

    class UDPTransport : public ITransport {
    public:
        static const uint16_t defaultPort;
        static const TransportTraits traits;

        explicit UDPTransport(std::shared_ptr<boost::asio::io_context> io_context,
                     IHostIdPtr hostId,
                     uint16_t port = defaultPort, bool v4 = true);

        uint16_t getPort() const;
        Endpoint getEndpoint() const;

        TransportTraits getTraits() const override;

        void extend(IExtensionPtr extension) override;

        cti::continuable<>
        connect(const NodeId &nodeId, const Endpoint &endpoint) override;

        ~UDPTransport();

    private:
        class Impl;

        std::unique_ptr<Impl> _impl;
    };

//    const TransportTraits UDPTransport::traits{
//            "UDP", false, true, false
//    };

//    const uint16_t UDPTransport::defaultPort{4080};
}

#endif //P2P_MSG_TRANSPORTS_UDP_H
