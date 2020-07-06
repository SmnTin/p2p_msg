#ifndef P2P_MSG_DETAILS_UDPSOCKET_H
#define P2P_MSG_DETAILS_UDPSOCKET_H

#include "EndpointTranslation.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace p2p::Basic::Network {
    class UDPSocket {
    public:
        typedef boost::asio::io_context Context;
        static const size_t MAX_UDP_PACKET_SIZE = 508u;

        UDPSocket(std::shared_ptr<Context> context,
                  uint16_t port, bool v4)
                : _port(port),
                  _socket(*context, UDPSocketEndpoint(
                          v4 ? boost::asio::ip::udp::v4()
                             : boost::asio::ip::udp::v6(), port)) {
            _context = std::move(context);
            _thisEndpoint = _socket.local_endpoint();

            _startReceiving();
        }

        void send(NodeId senderId, UDPSocketEndpoint recipientEndpoint, Buffer msg) {
            PacketHeader header;
            header.nodeId = senderId;
            std::size_t msgSize = msg.size();
            _assembleMsg(header, std::move(msg));
            _sendRaw(recipientEndpoint, msgSize + PacketHeader::size());
        }

        typedef std::function<void(NodeId senderNodeId,
                                   UDPSocketEndpoint senderEndpoint,
                                   Buffer msg)> ReceiveCallback;

        void setReceiveCallback(ReceiveCallback callback) {
            _receiveCallback = std::move(callback);
        }

        uint16_t getPort() const {
            return _port;
        }

        Endpoint getEndpoint() const {
            return UDPEndpointTranslation::serialize(_thisEndpoint);
        }

    private:
        uint8_t _sendBuf[MAX_UDP_PACKET_SIZE];
        uint8_t _receiveBuf[MAX_UDP_PACKET_SIZE];

        void _startReceiving() {
            _socket.async_receive_from(
                    boost::asio::buffer(_receiveBuf), _remoteEndpoint,
                    boost::bind(&UDPSocket::_receiveRaw, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
        }

        void _receiveRaw(const boost::system::error_code &error,
                         std::size_t receivedBytes) {
            if (!error || _checkReceivedMsgForCorrectness(receivedBytes)) {
                PacketHeader header = _extractHeader(receivedBytes);
                Buffer msg = _extractMsg(receivedBytes);
                _receiveCallback(header.nodeId, _remoteEndpoint, std::move(msg));
            }
            _startReceiving();
        }

        void _sendRaw(UDPSocketEndpoint socketEndpoint, std::size_t bytesToSend) {
            _socket.async_send_to(boost::asio::buffer(_sendBuf, bytesToSend), socketEndpoint,
                                  boost::bind(&UDPSocket::_handleSend, this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
        }

        void _handleSend(const boost::system::error_code &error,
                         std::size_t bytesTransferred) {}

        struct PacketHeader {
            NodeId nodeId;

            static size_t size() {
                return NodeId::sizeInBytes;
            }
        };

        bool _checkReceivedMsgForCorrectness(size_t receivedBytes) {
            return receivedBytes >= PacketHeader::size();
        }

        PacketHeader _extractHeader(size_t receivedBytes) {
            PacketHeader header;
            for (size_t i = 0; i < NodeId::sizeInBytes; ++i)
                header.nodeId[i] = networkToHostByteOrder(_receiveBuf[i]);
            return header;
        }

        Buffer _extractMsg(size_t receivedBytes) {
            return Buffer(_receiveBuf + PacketHeader::size(),
                          _receiveBuf + receivedBytes);
        }

        void _assembleMsg(PacketHeader header, Buffer msg) {
            if (msg.size() > MAX_UDP_PACKET_SIZE - PacketHeader::size())
                throw_p2p_exception("The message is too big for a single UDP packet.");
            for (size_t i = 0; i < NodeId::sizeInBytes; ++i)
                _sendBuf[i] = hostToNetworkByteOrder(header.nodeId[i]);
            std::copy(msg.begin(), msg.end(), _sendBuf + PacketHeader::size());
        }

        typedef boost::asio::ip::udp::socket RawSocket;
        uint16_t _port;
        RawSocket _socket;
        UDPSocketEndpoint _thisEndpoint;
        UDPSocketEndpoint _remoteEndpoint;
        std::shared_ptr<Context> _context;
        NodeId _thisNodeId;
        ReceiveCallback _receiveCallback;
    };
}

#endif //P2P_MSG_DETAILS_UDPSOCKET_H
