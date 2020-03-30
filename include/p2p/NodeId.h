#ifndef P2P_NODEID_H
#define P2P_NODEID_H

#include "p2p/config.h"
#include "p2p/platform.h"

#include "p2p/p2pAssert.h"

#include <stdint.h>
#include <array>
#include <functional>

namespace p2p {

	class NodeId {
	public:
		static constexpr std::size_t sizeInBytes = NODE_ID_SIZE_BYTES;
		static constexpr std::size_t sizeInBits = NODE_ID_SIZE_BITS;

		NodeId();
//		NodeId(NodeId & nodeId);

		static NodeId zeros();
		static NodeId max();
		static NodeId random();

		bool bitAt(std::size_t index) const;
		int firstLowBit() const;

		static std::size_t longestCommonPrefix(const NodeId & a, const NodeId & b);

		uint8_t operator[](std::size_t index) const;
		uint8_t& operator[](std::size_t index);

		bool operator==(const NodeId&) const;
		bool operator!=(const NodeId&) const;
		bool operator<(const NodeId&) const;

		/* A function to determine which of two NodeIds
		 * is closer to this NodeId with xor metrics.
		 */
		int xorCmp(const NodeId&, const NodeId&) const;

		NodeId operator^(const NodeId&) const;
		NodeId operator~() const;

		std::string hexStr() const;

		friend std::ostream & operator<< (std::ostream & out, const NodeId & nodeId);
		friend std::istream & operator>> (std::istream & in, NodeId & nodeId);

	private:
		std::array<uint8_t, NODE_ID_SIZE_BYTES> _data;
	};
}

namespace std {
	template <> struct hash<p2p::NodeId>
	{
		size_t operator()(const p2p::NodeId & nodeId) const
		{
			size_t res = 0;
			for(int i = 0; i < p2p::NodeId::sizeInBytes; ++i) {
				res *= NODE_ID_HASH_BASE;
				res += nodeId[i];
			}

			return res;
		}
	};
}

#endif
