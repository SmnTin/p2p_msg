#include "p2p/NodeId.h"

#include <boost/random/random_device.hpp>
#include <cstring>

namespace p2p {
	NodeId::NodeId() {
		_data.fill(0x00);
	}

	NodeId NodeId::zeros() {
		return NodeId();
	}

	NodeId NodeId::max() {
		return ~NodeId();
	}

	NodeId NodeId::random() {
        boost::random::random_device gen;
	    NodeId res;
        for(std::size_t i = 0; i < sizeInBytes; ++i)
            res[i] = (uint8_t)(gen()%255);
        return res;
	}

	bool NodeId::bitAt(std::size_t index) const {
		p2pAssert(index < sizeInBits, "NodeId::bitAt. Index must be less than size");

		const uint8_t& byte = _data.at(index / 8);
		const std::size_t bitIndex = 7 - index % 8;

		return ((1 << bitIndex) & byte) > 0;
	}

	int NodeId::firstLowBit() const {
		int i, j;
		for(i = sizeInBytes-1; i >= 0; i--)
			if(_data[i] != 0)
				break;
		if(i < 0)
			return -1;
		for(j = 7; j >= 0; j--)
			if((_data[i] & (0x80 >> j)) != 0)
				break;
		return 8 * i + j;
	}

	std::size_t NodeId::longestCommonPrefix(const p2p::NodeId &a, const p2p::NodeId &b) {
		for(std::size_t i = 0; i < sizeInBytes; ++i) {
			uint8_t _xor = a._data[i] ^ b._data[i];
			if(_xor == 0)
				continue;

			for(std::size_t  j = 0; j < 8; ++j) {
				int bitIndex = 7 - j;
				if(((1 << bitIndex) & _xor) > 0)
					return i*8 + j;
			}
		}
		return sizeInBits;
	}

	uint8_t NodeId::operator[](std::size_t index) const {
		return _data.at(index);
	}

	uint8_t& NodeId::operator[](std::size_t index) {
		return _data.at(index);
	}

	bool NodeId::operator==(const NodeId & other) const {
		return _data == other._data;
	}

	bool NodeId::operator!=(const NodeId & other) const {
		return _data != other._data;
	}

	bool NodeId::operator<(const NodeId & other) const {
		return _data < other._data;
	}

	NodeId NodeId::operator^(const NodeId & other) const {
		NodeId result;

		for(size_t i = 0; i < sizeInBytes; i++) {
			result._data[i] = other._data[i] ^ _data[i];
		}

		return result;
	}

	int NodeId::xorCmp(const NodeId& node1,const NodeId& node2) const {
		for(int i = 0; i < sizeInBytes; ++i) {
			if(node1[i] == node2[i])
				continue;
			uint8_t xor1 = node1[i] ^ _data[i],
					xor2 = node2[i] ^ _data[i];
			if(xor1 < xor2)
				return -1;
			else
				return 1;
		}
		return 0;
	}

	NodeId NodeId::operator~() const {
		NodeId result;

		for(size_t i = 0; i < sizeInBytes; i++) {
			result[i] = ~_data[i];
		}

		return result;
	}

    std::ostream & operator<< (std::ostream & out, const NodeId & nodeId) {
	    for(std::size_t i = 0; i < NodeId::sizeInBytes; ++i) {
            for (std::size_t j = 0; j < 8U; ++j)
                out << (std::string) (nodeId.bitAt(i*8+j) ? "1" : "0");
            if(i < NodeId::sizeInBytes - 1)
                out << (std::string)".";
        }
	    return out;
	}

	std::istream & operator>> (std::istream & in, NodeId & nodeId) {
		std::string s;
		in >> s;

		for(std::size_t i = 0; i < NodeId::sizeInBits + NodeId::sizeInBytes - 1; i += 9) {
			uint8_t block = 0;
			for (std::size_t j = 0; j < 8U; ++j)
				block |= ((s[i + j] - '0') << (7 - j));

			nodeId[i/9] = block;
		}

		return in;
	}
}
