#ifndef P2P_MSG_RUDPMESSAGES_H
#define P2P_MSG_RUDPMESSAGES_H

#include "p2p/Network/Message.h"
#include "p2p/Network/RUDP/Connection.h"

namespace p2p {
    namespace Network {
        namespace RUDP {

            constexpr Network::MessageLength RUDPHeaderLength = Network::LightHeaderLength + sizeof(ConnectionId);

            class RUDPMessage : public Network::LightMessage {
            public:
                Network::MessageLength constructHeader() override {
                    auto putLen = Network::LightMessage::constructHeader();

                    putInt<ConnectionId>(
                            std::next(_buff.begin(), putLen),
                            getConnectionId());
                    putLen += sizeof(MessageLength);

                    return putLen;
                }
                Network::MessageLength  parseHeader() override {
                    auto parsedLen = Network::LightMessage::parseHeader();

                    _connId = getInt<ConnectionId>(
                            std::next(_buff.begin(), parsedLen));
                    parsedLen += sizeof(ConnectionId);

                    return parsedLen;
                }
                ConnectionId getConnectionId() {
                    return _connId;
                }

            protected:
                ConnectionId _connId;
            };

            template<Network::MessageType type>
            class RUDPSimpleMessage : public RUDPMessage {
            public:

                explicit RUDPSimpleMessage(ConnectionId connId = 0) {
                    _buff.fill(0);
                    _flags = 0;
                    _type = type;
                    _length = RUDPHeaderLength;
                    _connId = connId;
                    constructHeader();
                }

                void parse(const Network::MessageBuffer & buff) {
                    _buff = buff;
                    Network::MessageLength parsedLen = parseHeader();
                }
            };

            constexpr Network::MessageType KEEP_ALIVE_MESSAGE_TYPE = 1;
            typedef RUDPSimpleMessage<KEEP_ALIVE_MESSAGE_TYPE> KeepAliveMessage;

            constexpr Network::MessageType OPEN_CONNECTION_REQUEST_MESSAGE_TYPE = 2;
            class OpenConnectionRequestMessage : public RUDPMessage {
            public:

                explicit OpenConnectionRequestMessage(ConnectionId connId = 0, const NodeId & nodeId = NodeId::zeros()) {
                    _buff.fill(0);
                    _flags = 0;
                    _type = OPEN_CONNECTION_REQUEST_MESSAGE_TYPE;
                    _length = RUDPHeaderLength + NodeId::sizeInBytes;
                    _connId = connId;
                    _nodeId = nodeId;
                    auto putLen = constructHeader();

                    for(size_t i = 0; i < NodeId::sizeInBytes; ++i)
                        _buff[putLen + i] = _nodeId[i];

                    putLen += NodeId::sizeInBytes;
                }

                void parse(const Network::MessageBuffer & buff) {
                    _buff = buff;
                    auto parsedLen = parseHeader();

                    for(size_t i = 0; i < NodeId::sizeInBytes; ++i)
                        _nodeId[i] = _buff[parsedLen + i];

                    parsedLen += NodeId::sizeInBytes;
                }

                NodeId getNodeId() {
                    return _nodeId;
                }

            private:
                NodeId _nodeId;
            };

            constexpr Network::MessageType OPEN_CONNECTION_RESPONSE_MESSAGE_TYPE = 3;
            class OpenConnectionResponseMessage : public RUDPMessage {
            public:

                explicit OpenConnectionResponseMessage(ConnectionId connId = 0, ConnectionId endpConnId = 0) {
                    _buff.fill(0);
                    _flags = 0;
                    _type = OPEN_CONNECTION_RESPONSE_MESSAGE_TYPE;
                    _length = RUDPHeaderLength + sizeof(ConnectionId);
                    _connId = connId;
                    _endpConnId = endpConnId;
                    auto putLen = constructHeader();

                    putInt<ConnectionId>(
                            std::next(_buff.begin(), putLen),
                            _endpConnId);
                    putLen += sizeof(ConnectionId);
                }

                void parse(const Network::MessageBuffer & buff) {
                    _buff = buff;
                    Network::MessageLength parsedLen = parseHeader();
                }

                ConnectionId getEndpointConnetionId() {
                    return _endpConnId;
                }

            private:
                ConnectionId _endpConnId;
            };

            constexpr Network::MessageType CLOSE_CONNECTION_MESSAGE_TYPE = 4;
            typedef RUDPSimpleMessage<CLOSE_CONNECTION_MESSAGE_TYPE> CloseConnectionMessage;

            constexpr Network::MessageFlags TERMINAL_SEQUENCE_FLAG = 0x02;
            constexpr Network::MessageType SEQUENCE_END_REQUEST_MESSAGE_TYPE = 8;
            class SequenceEndRequestMessage : public RUDPMessage {
            public:

                explicit SequenceEndRequestMessage(ConnectionId connId = 0, bool terminal = false) {
                    _buff.fill(0);
                    _flags = 0;
                    if(terminal)
                        _flags |= TERMINAL_SEQUENCE_FLAG;
                    _type = SEQUENCE_END_REQUEST_MESSAGE_TYPE;
                    _length = RUDPHeaderLength;
                    _connId = connId;
                    constructHeader();
                }

                void parse(const Network::MessageBuffer & buff) {
                    _buff = buff;
                    Network::MessageLength parsedLen = parseHeader();
                }

                bool isTerminal() {
                    return ((_flags & TERMINAL_SEQUENCE_FLAG) > 0);
                }
            };

            constexpr Network::MessageType SEQUENCE_END_RESPONSE_MESSAGE_TYPE = 9;
            typedef RUDPSimpleMessage<SEQUENCE_END_RESPONSE_MESSAGE_TYPE> SequenceEndResponseMessage;

            constexpr Network::MessageType ACKNOWLEDGE_REQUEST_MESSAGE_TYPE = 10;
            typedef RUDPSimpleMessage<ACKNOWLEDGE_REQUEST_MESSAGE_TYPE> AcknowledgeRequestMessage;

            constexpr Network::MessageType ACKNOWLEDGE_RESPONSE_MESSAGE_TYPE = 11;
            class AcknowledgeResponseMessage : public RUDPMessage {
            public:

                explicit AcknowledgeResponseMessage(ConnectionId connId = 0,
                        const std::bitset<RUDP_BLOCKS_IN_SEQUENCE> & receivedBlocks =
                                std::bitset<RUDP_BLOCKS_IN_SEQUENCE>()) {
                    _buff.fill(0);
                    _flags = 0;
                    _type = ACKNOWLEDGE_RESPONSE_MESSAGE_TYPE;
                    _length = RUDPHeaderLength + (RUDP_BLOCKS_IN_SEQUENCE + 7) / 8;
                    _connId = connId;
                    _receivedBlocks = receivedBlocks;
                    Network::MessageLength putLen = constructHeader();
                    std::cout << _receivedBlocks << "\n";
                    for(int i = 0; i < RUDP_BLOCKS_IN_SEQUENCE; ++i) {
                        if(_receivedBlocks[i])
                            _buff[putLen + i/8] |= (1 << (i % 8));
                    }
                }

                void parse(const Network::MessageBuffer & buff) {
                    _buff = buff;
                    Network::MessageLength parsedLen = parseHeader();

                    for(int i = 0; i < RUDP_BLOCKS_IN_SEQUENCE; ++i)
                        _receivedBlocks.set(i, ((buff[parsedLen + i/8] >> (i%8)) & 1) > 0);

                    std::cout << _receivedBlocks << "\n";
                }

                const std::bitset<RUDP_BLOCKS_IN_SEQUENCE> & getReceivedBlocks() {
                    return _receivedBlocks;
                }

            private:
                std::bitset<RUDP_BLOCKS_IN_SEQUENCE> _receivedBlocks;
            };

            constexpr Network::MessageLength MAX_BLOCK_DATA_SIZE =
                    MAX_UDP_PACKET_SIZE_BYTES - RUDPHeaderLength - sizeof(BlockIndex);
            constexpr Network::MessageType BLOCK_MESSAGE_TYPE = 12;
            class BlockMessage : public RUDPMessage {
            public:

                explicit BlockMessage(ConnectionId connId = 0,
                        const Network::MessageBuffer & val = Network::MessageBuffer(),
                        Network::MessageLength valLen = 0, BlockIndex blockInd = 0) {
                    _buff.fill(0);
                    _flags = 0;
                    _type = BLOCK_MESSAGE_TYPE;
                    _val = val;
                    _valLen = valLen;
                    _length = RUDPHeaderLength + sizeof(BlockIndex) + _valLen;
                    _connId = connId;
                    _blockInd = blockInd;

                    Network::MessageLength putLen = constructHeader();

                    putInt<BlockIndex>(
                            std::next(_buff.begin(), putLen),
                            _blockInd);
                    putLen += sizeof(BlockIndex);

                    for(int j = 0; j < _valLen; ++j)
                        *(std::next(_buff.begin(), putLen + j)) = *(std::next(_val.begin(), j));
                }

                void parse(const Network::MessageBuffer & buff) {
                    _buff = buff;
                    Network::MessageLength parsedLen = parseHeader();
                    _valLen = _length - RUDPHeaderLength - sizeof(BlockIndex);

                    _blockInd = getInt<BlockIndex>(
                            std::next(_buff.begin(), parsedLen));
                    parsedLen += sizeof(BlockIndex);

                    std::cout << "PARSED " << _blockInd << "\n";

                    for(int j = 0; j < _valLen; ++j)
                        *(std::next(_val.begin(), j)) = *(std::next(_buff.begin(), parsedLen + j));
                }

                Network::MessageLength getValLen() {
                    return _valLen;
                }

                Network::MessageBuffer getVal() {
                    return _val;
                }

                BlockIndex getBlockIndex() {
                    return _blockInd;
                }

            private:
                Network::MessageBuffer _val;
                Network::MessageLength _valLen;
                BlockIndex _blockInd;
            };

        } //RUDP
    } //Network
} //p2p

#endif //P2P_MSG_RUDPMESSAGES_H
