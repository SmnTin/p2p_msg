#ifndef P2P_MSG_MESSAGE_H
#define P2P_MSG_MESSAGE_H

#include <stdint.h>
#include <memory>

#include "p2p/Buffer.h"
#include "p2p/NodeId.h"

namespace p2p {
    namespace Network {

        typedef uint8_t MessageFlags;

        typedef uint8_t MessageType;

        typedef uint16_t MessageLength;

        typedef uint32_t MessageCounter; //also used as token

        //Every light message must have this header:
        /*
         *  _______________
         * |       |       |
         * | Flags | Type  |
         * |_______|_______|
         *
         */

        constexpr MessageLength LightHeaderLength =
                sizeof(MessageFlags) +
                sizeof(MessageType);

        class LightMessage {
        public:
            template<class T>
            static void putInt(MessageBuffer::iterator buffIt, T t) {
                for(int i = sizeof(T) - 1; i >= 0; --i) {
                    *buffIt = (t >> (i*8)) & 0xFF;
                    buffIt = std::next(buffIt);
                }
            }

            template<class T>
            static T getInt(MessageBuffer::const_iterator buffIt) {
                T t = 0;
                for(int i = sizeof(T) - 1; i >= 0; --i) {
                    t |= (((uint64_t)(*buffIt)) << (8*i));
                    buffIt = std::next(buffIt);
                }
                return t;
            }

            virtual MessageFlags getFlags() {
                return _flags;
            }

            virtual MessageType getType() {
                return _type;
            }

            virtual MessageLength getLength() {
                return _length;
            }

            virtual std::shared_ptr<MessageBuffer> getData() {
                return std::make_shared<MessageBuffer>(_buff);
            }

            virtual MessageLength constructHeader() {
                MessageLength putLength = 0;
                putInt(
                        std::next(_buff.begin(), putLength),
                        getFlags());
                putLength += sizeof(MessageFlags);

                putInt(
                        std::next(_buff.begin(), putLength),
                        getType());
                putLength += sizeof(MessageType);

                return putLength;
            }

            virtual MessageLength parseHeader() {
                MessageLength parsedLen = 0;
                _flags = getInt<MessageFlags>(
                        std::next(_buff.begin(), parsedLen));
                parsedLen += sizeof(MessageFlags);

                _type = getInt<MessageType>(
                        std::next(_buff.begin(), parsedLen));
                parsedLen += sizeof(MessageType);

                return parsedLen;
            }

        protected:
            MessageType _type;
            MessageFlags _flags;
            MessageBuffer _buff;
            MessageLength _length;
        };


        //Every message must have this header:
        /*
         *  _______________________________
         * |       |       |               |
         * | Flags | Type  |    Length     |
         * |_______|_______|_______________|
         * |                               |
         * |            Counter            |
         * |_______________________________|
         * |                               |
         * |           NodeId...           |
         * |_______________________________|
         * |                               |
         * |         ...NodeId,,,          |
         * |_______________________________|
         * |                               |
         * |         ...NodeId,,,          |
         * |_______________________________|
         * |                               |
         * |           ...NodeId           |
         * |_______________________________|
         *
         */

        constexpr MessageLength HeaderLength =
                sizeof(MessageFlags) +
                sizeof(MessageType) +
                sizeof(MessageLength) +
                sizeof(MessageCounter) +
                NODE_ID_SIZE_BYTES;


        class Message : public LightMessage {
        public:

            virtual MessageCounter getCounter() {
                return _counter;
            }

            virtual NodeId & getNodeId() {
                return _nodeId;
            }

        protected:
            MessageCounter _counter;
            NodeId _nodeId;

            MessageLength constructHeader() override {
                MessageLength putLength = 0;
                putInt(
                        std::next(_buff.begin(), putLength),
                        getFlags());
                putLength += sizeof(MessageFlags);

                putInt(
                        std::next(_buff.begin(), putLength),
                        getType());
                putLength += sizeof(MessageType);

                putInt(
                        std::next(_buff.begin(), putLength),
                        getLength());
                putLength += sizeof(MessageLength);

                putInt(
                        std::next(_buff.begin(), putLength),
                        getCounter());
                putLength += sizeof(MessageCounter);

                for(size_t i = 0; i < NodeId::sizeInBytes; ++i)
                    *std::next(_buff.begin(), putLength + i) = getNodeId()[i];

                putLength += NodeId::sizeInBytes;

                return putLength;
            }

            MessageLength parseHeader() override {
                MessageLength parsedLen = 0;
                _flags = getInt<MessageFlags>(
                        std::next(_buff.begin(), parsedLen));
                parsedLen += sizeof(MessageFlags);

                _type = getInt<MessageType>(
                        std::next(_buff.begin(), parsedLen));
                parsedLen += sizeof(MessageType);

                _length = getInt<MessageLength >(
                        std::next(_buff.begin(), parsedLen));
                parsedLen += sizeof(MessageLength);

                _counter = getInt<MessageCounter>(
                        std::next(_buff.begin(), parsedLen));
                parsedLen += sizeof(MessageCounter);

                for(size_t i = 0; i < NodeId::sizeInBytes; ++i)
                    _nodeId[i] = _buff[parsedLen + i];

                parsedLen += NodeId::sizeInBytes;

                return parsedLen;
            }
        };

    } // Network
} // p2p

#endif //P2P_MSG_MESSAGE_H
