#ifndef P2P_NETWORK_PACKET_H
#define P2P_NETWORK_PACKET_H

#include "p2p/config.h"
#include "p2p/p2pException.h"
#include "p2p/Network/Message.h"

#include <stdint.h>
#include <vector>

namespace p2p {
	namespace Network {
	    namespace RUDP {

            class Packet {
            public:
                Packet() {

                }

                std::vector<uint8_t> &data() {
                    return _data;
                }

                const std::vector<uint8_t> &data() const {
                    return _data;
                }

                uint8_t &operator[](size_t i) {
                    if (i < 0 || i >= _data.size())
                        throw_p2p_exception("Accessing out of the packet data.");
                    return _data[i];
                }

                uint8_t operator[](size_t i) const {
                    if (i < 0 || i >= _data.size())
                        throw_p2p_exception("Accessing out of the packet data.");
                    return _data[i];
                }

                size_t size() const {
                    return _data.size();
                }

                void append(const std::vector<uint8_t> &aData,
                        std::vector<uint8_t>::const_iterator bit, std::vector<uint8_t>::const_iterator eit) {
                    size_t curSize = _data.size();
                    if(eit - bit < 0)
                        return;
                    size_t len = eit - bit;
                    _data.resize(curSize + len);
                    std::copy(bit, eit, _data.begin() + curSize);
                }

                void append(const std::vector<uint8_t> &aData) {
                    size_t curSize = _data.size();
                    size_t len = aData.size();
                    _data.resize(curSize + aData.size());
                    std::copy(aData.begin(), aData.end(), _data.begin() + curSize);
                }

                void append(const Network::MessageBuffer &aData, size_t len) {
                    if (len > aData.size())
                        throw_p2p_exception("Accessing out of the appended data.");
                    size_t curSize = _data.size();
                    _data.resize(curSize + len);
                    std::copy(aData.begin(), std::next(aData.begin(), len), _data.begin() + curSize);
                }

                void append(const Packet & packet) {
                    size_t curSize = _data.size();
                    size_t len = packet.size();
                    _data.resize(curSize + packet.size());
                    std::copy(packet.data().begin(), packet.data().end(), _data.begin() + curSize);
                }

                void append(const std::shared_ptr<Packet> & packet) {
                    append(*packet);
                }

                MessageBuffer toMessageBuffer() {
                    if(_data.size() > MAX_UDP_PACKET_SIZE_BYTES)
                        throw_p2p_exception("FUCK YOU!");
                    MessageBuffer buffer;

                    for(int i = 0; i < _data.size(); ++i)
                        buffer[i] = _data[i];

                    return buffer;
                }

            private:
                std::vector<uint8_t> _data;
            };
        }
	}
}

#endif
