#include "EndpointTranslation.h"
#include "p2p/Common/Exceptions.h"

#include "TransportTraits.h"

namespace p2p::Basic::Network {
    typedef boost::asio::ip::udp::socket Socket;

    UDPSocketEndpoint UDPEndpointTranslation::deserialize(
            const Endpoint &endpoint) {
        checkEndpointForCorrectness(endpoint);
        boost::asio::ip::address address = extractAddressFromEndpoint(endpoint);
        uint16_t port = extractPortFromEndpoint(endpoint);

        return UDPSocketEndpoint(address, port);
    }

    Endpoint UDPEndpointTranslation::serialize(
            const UDPSocketEndpoint &socketEndpoint) {
        Endpoint endpoint;
        putTransportNameToEndpoint(endpoint);
        setSizeOfEndpoint(socketEndpoint, endpoint);
        putFamilyToEndpoint(socketEndpoint, endpoint);
        putAddressToEndpoint(socketEndpoint, endpoint);
        putPortToEndpoint(socketEndpoint, endpoint);

        return endpoint;
    }

//------------------------------------------------//

    void UDPEndpointTranslation::checkEndpointForCorrectness(const Endpoint &endpoint) {
        if (endpoint.transportName != UDPTransportTraits.getName())
            throw_p2p_exception("Incompatible transport type.");

        static const char *exceptionOfCorrupted = "Corrupted endpoint\n";
        if (endpoint.data.empty())
            throw_p2p_exception(exceptionOfCorrupted);

        uint8_t isV4 = endpoint.data[0];
        if (isV4 > 1 ||
            (isV4 && endpoint.data.size() != (1 + 4 + 2)) ||
            (!isV4 && endpoint.data.size() != (1 + 16 + 2)))
            throw_p2p_exception(exceptionOfCorrupted);
    }

    boost::asio::ip::address
    UDPEndpointTranslation::extractAddressFromEndpoint(const Endpoint &endpoint) {
        uint8_t isV4 = endpoint.data[0];
        boost::asio::ip::address address;
        if (isV4)
            address = extractAddressFromEndpoint_v4(endpoint);
        else
            address = extractAddressFromEndpoint_v6(endpoint);
        return address;
    }

    boost::asio::ip::address_v4
    UDPEndpointTranslation::extractAddressFromEndpoint_v4(const Endpoint &endpoint) {
        std::array<uint8_t, 4> rawAddr;
        std::copy(endpoint.data.begin() + 1,
                  endpoint.data.begin() + 5,
                  rawAddr.begin());
        return boost::asio::ip::address_v4(rawAddr);
    }

    boost::asio::ip::address_v6
    UDPEndpointTranslation::extractAddressFromEndpoint_v6(const Endpoint &endpoint) {
        std::array<uint8_t, 16> rawAddr;
        std::copy(endpoint.data.begin() + 1,
                  endpoint.data.begin() + 17,
                  rawAddr.begin());
        return boost::asio::ip::address_v6(rawAddr);
    }

    uint16_t UDPEndpointTranslation::extractPortFromEndpoint(const Endpoint &endpoint) {
        std::array<uint8_t, 2> rawPort;
        std::copy(endpoint.data.end() - 2,
                  endpoint.data.end(),
                  rawPort.begin());

        uint16_t portInNetworkByteOrder = *(uint16_t *) (rawPort.data());
        uint16_t port = ntohs(portInNetworkByteOrder);

        return port;
    }

//------------------------------------------------//

    void UDPEndpointTranslation::putTransportNameToEndpoint(Endpoint &endpoint) {
        endpoint.transportName = UDPTransportTraits.getName();
    }

    void UDPEndpointTranslation::setSizeOfEndpoint(const UDPSocketEndpoint &socketEndpoint,
                                                   Endpoint &endpoint) {
        if (socketEndpoint.address().is_v4())
            endpoint.data.resize(1 + 4 + 2);
        else
            endpoint.data.resize(1 + 16 + 2);
    }

    void UDPEndpointTranslation::putAddressToEndpoint(const UDPSocketEndpoint &socketEndpoint,
                                                      Endpoint &endpoint) {
        if (socketEndpoint.address().is_v4())
            putAddressToEndpoint_v4(socketEndpoint, endpoint);
        else
            putAddressToEndpoint_v6(socketEndpoint, endpoint);
    }

    void UDPEndpointTranslation::putAddressToEndpoint_v4(const UDPSocketEndpoint &socketEndpoint,
                                                         Endpoint &endpoint) {
        auto rawAddr = socketEndpoint.address().to_v4().to_bytes();
        std::copy(rawAddr.begin(),
                  rawAddr.end(),
                  endpoint.data.begin() + 1);
    }

    void UDPEndpointTranslation::putAddressToEndpoint_v6(const UDPSocketEndpoint &socketEndpoint,
                                                         Endpoint &endpoint) {
        auto rawAddr = socketEndpoint.address().to_v6().to_bytes();
        std::copy(rawAddr.begin(),
                  rawAddr.end(),
                  endpoint.data.begin() + 1);
    }

    void UDPEndpointTranslation::putFamilyToEndpoint(const UDPSocketEndpoint &socketEndpoint,
                                                     Endpoint &endpoint) {
        endpoint.data[0] = socketEndpoint.address().is_v4();
    }

    void UDPEndpointTranslation::putPortToEndpoint(const UDPSocketEndpoint &socketEndpoint,
                                                   Endpoint &endpoint) {
        uint16_t portInHostByteOrder = socketEndpoint.port();
        uint16_t port = htons(portInHostByteOrder);
        *(endpoint.data.end() - 2) = ((uint8_t *) &port)[0];
        *(endpoint.data.end() - 1) = ((uint8_t *) &port)[1];
    }
}