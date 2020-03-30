#ifndef P2P_NETWORK_BUFFER_H
#define P2P_NETWORK_BUFFER_H

#include <stdint.h>
#include <vector>
#include <array>

#include "p2p/config.h"

namespace p2p {

	typedef std::vector<uint8_t> Buffer;

	namespace Network {
		typedef std::array<uint8_t, MAX_UDP_PACKET_SIZE_BYTES> MessageBuffer;
	}
}
#endif
