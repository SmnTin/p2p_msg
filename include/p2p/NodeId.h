#ifndef P2P_NODEID_H
#define P2P_NODEID_H

#include "p2p/platform.h"

#include "p2p/p2pAssert.h"

#include <stdint.h>
#include <array>
#include <functional>

namespace p2p {

	class NodeId {
	public:
		static constexpr std::size_t sizeInBytes = 32;
		static constexpr std::size_t sizeInBits = sizeInBytes * 8;
		typedef std::array<uint8_t, sizeInBytes> Data;

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
		Data data() const;

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
	    Data _data;
	};
}

namespace std {
	template <> struct hash<p2p::NodeId>
	{
		size_t operator()(const p2p::NodeId & nodeId) const
		{
		    static const int base = 123;
		    static const int mod = 1e9 + 7;
			size_t res = 0;
			for(int i = 0; i < p2p::NodeId::sizeInBytes; ++i) {
				res *= base;
				res += nodeId[i];
				res %= mod;
			}

			return res;
		}
	};
}

#endif
