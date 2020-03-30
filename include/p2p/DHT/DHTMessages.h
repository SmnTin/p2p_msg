#ifndef P2P_MSG_DHTMESSAGES_H
#define P2P_MSG_DHTMESSAGES_H

#include <memory>

#include "p2p/Network/Message.h"
#include "p2p/NodeId.h"

namespace p2p {
    namespace DHT {
        constexpr Network::MessageFlags DHT_MESSAGES_FLAG = 0x01;

        constexpr Network::MessageType PING_MESSAGE_TYPE = 1;
        class PingMessage : public Network::Message {
        public:

            explicit PingMessage(Network::MessageCounter counter = 0, const NodeId & id = NodeId::zeros()) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = id;
                _flags = DHT_MESSAGES_FLAG;
                _type = PING_MESSAGE_TYPE;
                _length = Network::HeaderLength;
                constructHeader();
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();
            }

        };

        constexpr Network::MessageType PONG_MESSAGE_TYPE = 2;
        class PongMessage : public Network::Message {
        public:

            explicit PongMessage(Network::MessageCounter counter = 0, const NodeId & id = NodeId::zeros()) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = id;
                _flags = DHT_MESSAGES_FLAG;
                _type = PONG_MESSAGE_TYPE;
                _length = Network::HeaderLength;

                constructHeader();
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();
            }

        };

        constexpr Network::MessageType FIND_NODE_REQUEST_MESSAGE_TYPE = 3;
        class FindNodeRequestMessage : public Network::Message {
        public:

            NodeId getDestNodeId() {
                return _destNodeId;
            }

            explicit FindNodeRequestMessage(Network::MessageCounter counter = 0,
                    const NodeId & myId = NodeId::zeros(),
                    const NodeId & destId = NodeId::zeros()) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = myId;
                _flags = DHT_MESSAGES_FLAG;
                _type = FIND_NODE_REQUEST_MESSAGE_TYPE;
                _length = Network::HeaderLength + NODE_ID_SIZE_BYTES;

                constructHeader();

                for(int j = 0; j < NODE_ID_SIZE_BYTES; ++j)
                    *(std::next(_buff.begin(), Network::HeaderLength + j)) = destId[j];
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();

                for(int j = 0; j < NODE_ID_SIZE_BYTES; ++j)
                    _destNodeId[j] = *(std::next(_buff.begin(), Network::HeaderLength + j));
            }
        private:
            NodeId _destNodeId;
        };

        constexpr int NODE_SIZE = (NODE_ID_SIZE_BYTES + 6);
        constexpr int MAX_NODES_IN_FIND_NODE_RESPONSE =
                (MAX_UDP_PACKET_SIZE_BYTES - Network::HeaderLength) / NODE_SIZE;


        constexpr Network::MessageType FIND_NODE_RESPONSE_MESSAGE_TYPE = 4;
        class FindNodeResponseMessage : public Network::Message {
        public:

            explicit FindNodeResponseMessage(Network::MessageCounter counter = 0,
                                            const NodeId & myId = NodeId::zeros(),
                                            std::vector<std::shared_ptr<Node>> data = {}) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = myId;
                _flags = DHT_MESSAGES_FLAG;
                _type = FIND_NODE_RESPONSE_MESSAGE_TYPE;
                _length = Network::HeaderLength +
                        (Network::MessageLength)(std::min((int)data.size(), MAX_NODES_IN_FIND_NODE_RESPONSE) * NODE_SIZE);

                constructHeader();

                for(int i = 0; i < std::min((int)data.size(), MAX_NODES_IN_FIND_NODE_RESPONSE); ++i) {
                    Network::MessageLength stPoint = Network::HeaderLength +
                            (Network::MessageLength)(i*NODE_SIZE);

                    for(int j = 0; j < NODE_ID_SIZE_BYTES; ++j)
                        *(std::next(_buff.begin(), stPoint + j)) = data[i]->id()[j];

                    uint32_t addr = (uint32_t)data[i]->address().address().to_v4().to_ulong();
                    uint16_t port = data[i]->address().port();
                    for(int j = 3; j >= 0; --j) {
                        *(std::next(_buff.begin(), stPoint + NODE_ID_SIZE_BYTES + j)) =
                                (uint8_t)((addr >> (8*j)) & 0xFF);
                    }
                    for(int j = 1; j >= 0; --j) {
                        *(std::next(_buff.begin(), stPoint + NODE_ID_SIZE_BYTES + 4 + j)) =
                                (uint8_t)((port >> (8*j)) & 0xFF);
                    }
                }
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();

                int cnt = (_length - Network::HeaderLength) / NODE_SIZE;

                for(int i = 0; i < std::min(cnt, MAX_NODES_IN_FIND_NODE_RESPONSE); ++i) {
                    Network::MessageLength stPoint = Network::HeaderLength +
                                                     (Network::MessageLength)(i*NODE_SIZE);

                    NodeId id;
                    for(int j = 0; j < NODE_ID_SIZE_BYTES; ++j)
                        id[j] = *(std::next(_buff.begin(), stPoint + j));

                    uint32_t addr = 0;
                    uint16_t port = 0;
                    for(int j = 3; j >= 0; --j) {
                        addr |= (((uint32_t )*(std::next(_buff.begin(), stPoint + NODE_ID_SIZE_BYTES + j))) << 8*j);
                    }
                    for(int j = 1; j >= 0; --j) {
                        port |= (((uint16_t )*(std::next(_buff.begin(), stPoint + 4 + NODE_ID_SIZE_BYTES + j))) << 8*j);
                    }

                    boost::asio::ip::udp::endpoint endp (
                            boost::asio::ip::address(boost::asio::ip::address_v4((unsigned long)addr)), port);

                    _data.push_back(std::make_shared<Node>(id, endp));
                }
            }

            std::vector<std::shared_ptr<Node>> getNodes() {
                return _data;
            }

        private:
            std::vector<std::shared_ptr<Node>> _data;
        };

        typedef Network::MessageBuffer StorageValueBuffer;
        constexpr Network::MessageLength MAX_STORAGE_VALUE_BUFFER_SIZE =
                MAX_UDP_PACKET_SIZE_BYTES - NodeId::sizeInBytes - Network::HeaderLength;
        constexpr Network::MessageType STORE_VALUE_REQUEST_MESSAGE_TYPE = 5;
        class StoreValueRequestMessage : public Network::Message {
        public:
            explicit StoreValueRequestMessage(Network::MessageCounter counter = 0,
                    const NodeId & id = NodeId::zeros(),
                    const NodeId & key = NodeId::zeros(),
                    const StorageValueBuffer & val = StorageValueBuffer(),
                    const Network::MessageLength valLen = MAX_STORAGE_VALUE_BUFFER_SIZE) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = id;
                _flags = DHT_MESSAGES_FLAG;
                _type = STORE_VALUE_REQUEST_MESSAGE_TYPE;
                _length = Network::HeaderLength + NodeId::sizeInBytes + valLen;
                constructHeader();

                _key = key;
                _val = val;

                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    *(std::next(_buff.begin(), Network::HeaderLength + j)) = _key[j];

                for(int j = 0; j < valLen; ++j)
                    *(std::next(_buff.begin(), Network::HeaderLength + NodeId::sizeInBytes + j)) =
                            *(std::next(_val.begin(), j));

//                std::cout << "STORE REQ " << _key << " " << (int)_val[0] << "\n";
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();
                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    _key[j] = *(std::next(_buff.begin(), parsedLen + j));
                parsedLen += NodeId::sizeInBytes;

                for(int j = 0; parsedLen + j < _length; ++j)
                    *(std::next(_val.begin(), j)) = *(std::next(_buff.begin(), parsedLen + j));

//                std::cout << "STORE REQ " << _key << " " << (int)_val[0] << "\n";
            }

            const NodeId getKey() {
                return _key;
            }

            const Network::MessageBuffer getVal() {
                return _val;
            }

        private:
            NodeId _key;
            StorageValueBuffer _val;
        };

        constexpr Network::MessageType STORE_VALUE_RESPONSE_MESSAGE_TYPE = 6;
        class StoreValueResponseMessage : public Network::Message {
        public:
            explicit StoreValueResponseMessage(Network::MessageCounter counter = 0,
                                              const NodeId & id = NodeId::zeros(),
                                              const NodeId & key = NodeId::zeros()) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = id;
                _flags = DHT_MESSAGES_FLAG;
                _type = STORE_VALUE_RESPONSE_MESSAGE_TYPE;
                _length = Network::HeaderLength + NodeId::sizeInBytes;
                constructHeader();

                _key = key;

                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    *(std::next(_buff.begin(), Network::HeaderLength + j)) = _key[j];

//                std::cout << "STORE RES " << _key << "\n";
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();
                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    _key[j] = *(std::next(_buff.begin(), parsedLen + j));

//                std::cout << "STORE RES " << _key << "\n";
            }

            const NodeId getKey() {
                return _key;
            }

        private:
            NodeId _key;
        };

        constexpr Network::MessageType GET_VALUE_REQUEST_MESSAGE_TYPE = 7;
        class GetValueRequestMessage : public Network::Message {
        public:
            explicit GetValueRequestMessage(Network::MessageCounter counter = 0,
                                            const NodeId & id = NodeId::zeros(),
                                            const NodeId & key = NodeId::zeros()) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = id;
                _flags = DHT_MESSAGES_FLAG;
                _type = GET_VALUE_REQUEST_MESSAGE_TYPE;
                _length = Network::HeaderLength + NodeId::sizeInBytes;
                constructHeader();

                _key = key;

                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    *(std::next(_buff.begin(), Network::HeaderLength + j)) = _key[j];

//                std::cout << "GET REQ " << _key << "\n";
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();
                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    _key[j] = *(std::next(_buff.begin(), parsedLen + j));

//                std::cout << "GET REQ " << _key << "\n";
            }

            const NodeId getKey() {
                return _key;
            }

        private:
            NodeId _key;
        };

        const Network::MessageFlags GET_VALUE_SUCCESS_FLAG = 0x02;

        constexpr Network::MessageType GET_VALUE_RESPONSE_MESSAGE_TYPE = 8;
        class GetValueResponseMessage : public Network::Message {
        public:
            explicit GetValueResponseMessage(Network::MessageCounter counter = 0,
                                             const NodeId & id = NodeId::zeros(),
                                             const NodeId & key = NodeId::zeros(),
                                             const StorageValueBuffer & val = Network::MessageBuffer(),
                                             bool success = false) {
                _buff.fill(0);
                _counter = counter;
                _nodeId = id;
                _flags = DHT_MESSAGES_FLAG;
                if(success)
                    _flags |= GET_VALUE_SUCCESS_FLAG;
                _type = GET_VALUE_RESPONSE_MESSAGE_TYPE;
                _length = MAX_UDP_PACKET_SIZE_BYTES;
                constructHeader();

                _key = key;
                _val = val;

                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    *(std::next(_buff.begin(), Network::HeaderLength + j)) = _key[j];

                for(int j = 0; j < MAX_UDP_PACKET_SIZE_BYTES - Network::HeaderLength - NodeId::sizeInBytes; ++j)
                    *(std::next(_buff.begin(), Network::HeaderLength + NodeId::sizeInBytes + j)) =
                            *(std::next(_val.begin(), j));

//                std::cout << "GET RES " << _key << " " << (int)_val[0] << "\n";
            }

            void parse(const Network::MessageBuffer & buff) {
                _buff = buff;
                Network::MessageLength parsedLen = parseHeader();
                for(int j = 0; j < NodeId::sizeInBytes; ++j)
                    _key[j] = *(std::next(_buff.begin(), parsedLen + j));
                parsedLen += NodeId::sizeInBytes;

                for(int j = 0; parsedLen + j < _length; ++j)
                    *(std::next(_val.begin(), j)) = *(std::next(_buff.begin(), parsedLen + j));
//                std::cout << "GET RES " << _key << " " << (int)_val[0] << "\n";
            }

            const NodeId getKey() {
                return _key;
            }

            bool isSuccessful() {
                return ((_flags & GET_VALUE_SUCCESS_FLAG) > 0);
            }

            const Network::MessageBuffer getVal() {
                return _val;
            }

        private:
            NodeId _key;
            StorageValueBuffer _val;
        };

    } //DHT
}
#endif //P2P_MSG_DHTMESSAGES_H
