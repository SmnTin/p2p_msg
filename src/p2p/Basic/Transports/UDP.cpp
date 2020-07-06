#include "p2p/Basic/Transports/UDP.h"

#include "p2p/Basic/Network/Extension.h"
#include "p2p/Basic/Network/Stream.h"

#include "details/UDP/TransportTraits.h"
#include "details/UDP/Socket.h"

#include <unordered_map>
#include <boost/bind.hpp>

namespace p2p::Basic::Network {
    using namespace p2p::Network;
    typedef UDPSocket::Context Context;

    const TransportTraits UDPTransport::traits = UDPTransportTraits;
    const uint16_t UDPTransport::defaultPort{4080};

    class UDPTransport::Impl {
    public:
        struct ConnectionInfo {
            NodeId nodeId;
            Endpoint endpoint;
            UDPSocketEndpoint socketEndpoint; //internal representation
        };
        typedef std::unique_ptr<ConnectionInfo> ConnectionInfoUPtr;

        class UDPStream : public Stream<Messaging::NoPolicy> {
        public:
            UDPStream(Impl *implPtr, ConnectionInfo *connectionInfoPtr)
                    : _implPtr(implPtr), _infoPtr(connectionInfoPtr) {}

            void send(Buffer msg) override {
                _implPtr->_send(_infoPtr, std::move(msg));
            }

            NodeId getNodeId() override {
                return _infoPtr->nodeId;
            }

            TransportTraits getTraits() override {
                return UDPTransportTraits;
            }

            Endpoint getEndpoint() override {
                return _infoPtr->endpoint;
            }

        private:
            Impl *_implPtr;
            ConnectionInfo *_infoPtr;
        };

        typedef std::shared_ptr<UDPStream> UDPStreamPtr;

        struct ConnectionRecord {
            ConnectionInfoUPtr info;
            UDPStreamPtr stream;
        };

        Impl(std::shared_ptr<Context> io_context,
             IHostIdPtr hostId,
             uint16_t port, bool v4)
                : _socket(io_context, port, v4) {
            _hostId = std::move(hostId);
            _context = std::move(io_context);
            _socket.setReceiveCallback(std::bind(
                    &Impl::_handleReceive, this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3));
        }

        uint16_t getPort() const {
            return _socket.getPort();
        }

        Endpoint getEndpoint() const {
            return _socket.getEndpoint();
        }

        TransportTraits getTraits() const {
            return UDPTransport::traits;
        }

        void extend(IExtensionPtr extension) {
            _extensionPtr = extension;
        }

        cti::continuable<>
        connect(const NodeId &nodeId, const Endpoint &endpoint) {
            auto &record = _createRecord(nodeId, endpoint, UDPEndpointTranslation::deserialize(endpoint));
            record.stream->performHandshake();

            return cti::make_ready_continuable();
        }

    private:

        ConnectionRecord &_createRecord(const NodeId &nodeId,
                                        const Endpoint &endpoint,
                                        const UDPSocketEndpoint &socketEndpoint) {
            if (_routingTable.count(nodeId))
                throw_p2p_exception("The record already exists.");

            ConnectionRecord &record = _routingTable[nodeId];

            record.info = std::make_unique<ConnectionInfo>();
            record.info->nodeId = nodeId;
            record.info->endpoint = endpoint;
            record.info->socketEndpoint = socketEndpoint;

            record.stream = std::make_shared<UDPStream>(this, record.info.get());
            if (_extensionPtr)
                _extensionPtr->extendStream(record.stream);

            return record;
        }

        void _send(ConnectionInfo *connectionInfoPtr, Buffer msg) {
            _socket.send(_hostId->getId(),
                         connectionInfoPtr->socketEndpoint,
                         std::move(msg));
        }

        void _handleReceive(NodeId senderNodeId,
                            UDPSocketEndpoint senderEndpoint,
                            Buffer msg) {
            if (!_routingTable.count(senderNodeId))
                _createRecord(senderNodeId,
                              UDPEndpointTranslation::serialize(senderEndpoint),
                              senderEndpoint);
            auto &record = _routingTable[senderNodeId];

            if (record.info->socketEndpoint != senderEndpoint) {
                record.info->socketEndpoint = senderEndpoint;
                record.info->endpoint = UDPEndpointTranslation::serialize(senderEndpoint);
            }

            _receive(record.stream, std::move(msg));
        }

        void _receive(IStreamPtr stream, Buffer msg) {
            try {
                stream->receive(std::move(msg));
            } catch (...) {
                //ignore the exception
                //there is nobody further to handle it
            }
        }

        UDPSocket _socket;
        std::shared_ptr<Context> _context;

        IHostIdPtr _hostId;

        IExtensionPtr _extensionPtr;

        std::unordered_map<NodeId, ConnectionRecord> _routingTable;
    };

    UDPTransport::UDPTransport(std::shared_ptr<Context> io_context,
                               IHostIdPtr hostId,
                               uint16_t port, bool v4)
            : _impl(std::make_unique<Impl>(std::move(io_context), hostId, port, v4)) {}

    uint16_t UDPTransport::getPort() const {
        return _impl->getPort();
    }

    Endpoint UDPTransport::getEndpoint() const {
        return _impl->getEndpoint();
    }

    TransportTraits UDPTransport::getTraits() const {
        return _impl->getTraits();
    }

    cti::continuable<> UDPTransport::connect(const NodeId &nodeId, const Endpoint &endpoint) {
        return _impl->connect(nodeId, endpoint);
    }

    void UDPTransport::extend(IExtensionPtr extension) {
        _impl->extend(extension);
    }

    UDPTransport::~UDPTransport() = default;
}