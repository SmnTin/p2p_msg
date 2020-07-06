#include "p2p/Common/Endianness.h"
#include "p2p/Network/Endpoint.h"

#include "boost/asio.hpp"

/*---------_UDP Endpoint Data Format----------
|------|------...........------|-------------|
|1 byte|     4 or 16 bytes     |   2 bytes   |
|------|------...........------|-------------|
|1/0 if|  ipv4 or ipv6 address |     port    |
|it is |  written as-is        |  written in |
|ipv4  |  one by one: a, b...  |  big-endian |
|------|------...........------|-------------|*/

namespace p2p::Basic::Network {
    using namespace p2p::Network;

    typedef boost::asio::ip::udp::endpoint UDPSocketEndpoint;

    class UDPEndpointTranslation {
    public:
        static UDPSocketEndpoint deserialize(const Endpoint &endpoint);
        static Endpoint serialize(const UDPSocketEndpoint &socketEndpoint);
    private:
        static void checkEndpointForCorrectness(const Endpoint &endpoint);
        static boost::asio::ip::address extractAddressFromEndpoint(const Endpoint &endpoint);
        static boost::asio::ip::address_v4 extractAddressFromEndpoint_v4(const Endpoint &endpoint);
        static boost::asio::ip::address_v6 extractAddressFromEndpoint_v6(const Endpoint &endpoint);
        static uint16_t extractPortFromEndpoint(const Endpoint &endpoint);

        static void putTransportNameToEndpoint(Endpoint &endpoint);
        static void setSizeOfEndpoint(const UDPSocketEndpoint &socketEndpoint, Endpoint &endpoint);
        static void putAddressToEndpoint(const UDPSocketEndpoint &socketEndpoint, Endpoint &endpoint);
        static void putAddressToEndpoint_v4(const UDPSocketEndpoint &socketEndpoint, Endpoint &endpoint);
        static void putAddressToEndpoint_v6(const UDPSocketEndpoint &socketEndpoint, Endpoint &endpoint);
        static void putFamilyToEndpoint(const UDPSocketEndpoint &socketEndpoint, Endpoint &endpoint);
        static void putPortToEndpoint(const UDPSocketEndpoint &socketEndpoint, Endpoint &endpoint);
    };
}