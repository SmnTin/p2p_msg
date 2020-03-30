#include <p2p/Network/RUDP/RUDPMessages.h>
#include "p2p/Network/RUDP/Connection.h"
#include "p2p/Network/RUDP/Engine.h"

namespace p2p {
    namespace Network {
        namespace RUDP {

            Connection::Connection() = default;
            Connection::Connection(p2p::Network::RUDP::Engine *engine, p2p::Network::RUDP::ConnectionId connId,
                                   std::shared_ptr<p2p::Node> node) :
                                   _node(std::move(node)), _engine(engine), _connId(connId) {}

            Connection::~Connection() {
                std::lock_guard<std::recursive_mutex> guard1(_sendMutex);
                std::lock_guard<std::recursive_mutex> guard2(_receiveMutex);

                if(_responseJob)
                    _responseJob->cancel();
                if(_connectionTimeoutJob)
                    _connectionTimeoutJob->cancel();
                if(_pingJob)
                    _pingJob->cancel();
            }

            void Connection::connect() {
                std::function<void()> onEvery = [this](){
                    _engine->getNetworkEngine()->sendMessage(
                            std::make_shared<OpenConnectionRequestMessage>(_connId,
                                    _engine->getDHT()->getRoutingTable()->getId()), _node->address());
                };
                std::function<void()> onFail = std::bind(&Connection::_onConnectionTimeout, this);

                makeAttempts(onEvery, onFail, RUDP_CONNECTION_ATTEMPTS);
            }

            void Connection::acceptConnection(ConnectionId endpConnId) {
                std::cout << "RUDP: Connection!\n";
                if(onConnectionCallback)
                    onConnectionCallback();
                _endpConnId = endpConnId;
                onKeepAlive();
            }

            void Connection::disconnect() {
                _engine->getNetworkEngine()->sendMessage(std::make_shared<CloseConnectionMessage>(_endpConnId),
                        _node->address());

                _pingJob->cancel();
            }

            void Connection::onDisconnection() {
                std::cout << "RUDP: Disconnection!\n";
                if(onDisconnectionCallback)
                    onDisconnectionCallback();
                disconnect();
            }

            void Connection::_onConnectionTimeout()  {
//                disconnect();
            }

            std::shared_ptr<Node> Connection::getNode() {
                return _node;
            }

            ConnectionId Connection::getConnectionId() {
                return _connId;
            }

            ConnectionId Connection::getEndpointConnectionId() {
                return _endpConnId;
            }

            void Connection::onResponseMessage() {
                auto now = std::chrono::steady_clock::now();
                stopAttempts();
                if(_anySent) {
                    _RTT = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastSent) * 0.125
                                    + _RTT * 0.875);
                }
            }

            void Connection::onEachMessage(boost::asio::ip::udp::endpoint &endp) {
                _node->heard(time(0));
                _node->address() = endp;
//                std::cout << "RUDP: Recieved message" << std::endl;
                if(_connectionTimeoutJob)
                    _connectionTimeoutJob->cancel();
                _connectionTimeoutJob = _engine->getSheduler()->executeTaskIn(
                        std::chrono::milliseconds {RUDP_CONNECTION_TIMEOUT_IN_SECONDS * 1000},
                        std::bind(&Connection::_onConnectionTimeout, this));
            }

            void Connection::makeAttempts(std::function<void()> onEvery, std::function<void()> onFail, int maxAttempts,
                                          int attempts) {
                if(maxAttempts == -1 || attempts <= maxAttempts) {
                    onEvery();

                    _responseJob = _engine->getSheduler()->executeTaskIn(attempts*_RTT, std::bind(
                            &Connection::makeAttempts, this, onEvery, onFail, maxAttempts, attempts + 1));
                } else
                    onFail();
            }

            void Connection::stopAttempts() {
                if(_responseJob)
                    _responseJob->cancel();
            }

            void Connection::onConnectionResponse() {
//                stopAttempts();
            }

            void Connection::onSequenceEndRequest(bool terminal) {
                std::lock_guard<std::recursive_mutex> guard(_receiveMutex);
                std::shared_ptr<Packet> sequence(new Packet());

                for(int i = 0; i < _receivedBlocks.count(); ++i)
                    sequence->append(_sequenceReceived[i]);

                _sequencesReceived.push(sequence);
                _receivedBlocks.reset();
                _sequenceReceived.fill(std::make_shared<Packet>());

                if(terminal) {
                    std::shared_ptr<Packet> packet(new Packet());
                    while(!_sequencesReceived.empty()) {
                        packet->append(_sequencesReceived.front());
                        _sequencesReceived.pop();
                    }

                    if(onMessageCallback)
                        onMessageCallback(packet);
                }

                _engine->getNetworkEngine()->sendMessage(std::make_shared<SequenceEndResponseMessage> (_endpConnId),
                        _node->address());
            }

            void Connection::onBlock(const p2p::Network::RUDP::BlockIndex &ind, const p2p::Network::MessageBuffer &val,
                                     const p2p::Network::MessageLength &valLen) {
                std::lock_guard<std::recursive_mutex> guard(_receiveMutex);
                if(ind >= RUDP_BLOCKS_IN_SEQUENCE)
                    return;
                std::shared_ptr<Packet> block(new Packet());
                std::cout << "Got it\n";

                block->append(val, valLen);
                _receivedBlocks.set(ind);
                _sequenceReceived[ind] = block;
            }

            void Connection::setMessageCallback(const MessageCallback &callback) {
                onMessageCallback = callback;
            }
            void Connection::setConnectionCallback(const ConnectionCallback &callback) {
                onConnectionCallback = callback;
            }
            void Connection::setDisconnectionCallback(const DisconnectionCallback &callback) {
                onDisconnectionCallback = callback;
            }

            void Connection::onAckRequest() {
                std::cout << "Got ack.\n";
                std::lock_guard<std::recursive_mutex> guard(_receiveMutex);
                _engine->getNetworkEngine()->sendMessage(std::make_shared<AcknowledgeResponseMessage>(
                        _endpConnId, _receivedBlocks), _node->address());
            }

            void Connection::onAckResponse(const std::bitset<RUDP_BLOCKS_IN_SEQUENCE> &receivedBlocks) {
                std::lock_guard<std::recursive_mutex> guard(_sendMutex);
                _sentBlocks = receivedBlocks;
                sendSequence();
            }

            void Connection::onSequenceEndResponse() {
                sendNextSequence();
            }

            void Connection::send(const std::shared_ptr<Packet> &packet) {
                std::lock_guard<std::recursive_mutex> guard(_sendMutex);
                _packetsToSend.push(packet);
                if(!_isSending)
                    sendPacket();
            }

            void Connection::sendSequence() {
                std::lock_guard<std::recursive_mutex> guard(_sendMutex);
                if(_sentBlocks.count() < _blocksInSequence) {
                    for(int i = 0; i < _blocksInSequence; ++i) {
                        if(!_sentBlocks[i])
                            _engine->getNetworkEngine()->sendMessage(std::make_shared<BlockMessage>(
                                    _endpConnId, _sequenceToSend[i]->toMessageBuffer(), _sequenceToSend[i]->size(), i),
                                            _node->address());
                    }
                    std::function<void()> onEvery = [this]() {
                        _anySent = true;
                        _engine->getNetworkEngine()->sendMessage(std::make_shared<AcknowledgeRequestMessage> (
                                _endpConnId), _node->address());
                    };
                    makeAttempts(onEvery, std::bind(&Connection::_onConnectionTimeout, this));
                } else {
                    std::function<void()> onEvery = [this]() {
                        _anySent = true;
                        _engine->getNetworkEngine()->sendMessage(std::make_shared<SequenceEndRequestMessage> (
                                _endpConnId, _sequencesToSend.empty()), _node->address());
                    };
                    makeAttempts(onEvery, std::bind(&Connection::_onConnectionTimeout, this));
                }
            }

            void Connection::sendNextSequence() {
                std::lock_guard<std::recursive_mutex> guard(_sendMutex);
                if(_sequencesToSend.empty()) {
                    sendPacket();
                    return;
                }
                _sentBlocks.reset();
                for(auto & seq : _sequenceToSend)
                    seq = std::make_shared<Packet>();
                auto curSeq = _sequencesToSend.front();
                _sequencesToSend.pop();
                _blocksInSequence = 0;

                for(int i = 0; i < RUDP_BLOCKS_IN_SEQUENCE; ++i) {
                    _sequenceToSend[i]->append(curSeq->data(),
                            std::min(curSeq->data().end(), std::next(curSeq->data().begin(), (i) * MAX_BLOCK_DATA_SIZE)),
                            std::min(curSeq->data().end(), std::next(curSeq->data().begin(), (i+1) * MAX_BLOCK_DATA_SIZE)));

                    if(_sequenceToSend[i]->size() > 0)
                        _blocksInSequence++;
                }

                sendSequence();
            }

            void Connection::sendPacket() {
                if(_packetsToSend.empty()) {
                    _isSending = false;
                    return;
                }

                _isSending = true;
                auto & curPacket = _packetsToSend.front();
                _packetsToSend.pop();

                for(int i = 0; i * MAX_BLOCK_DATA_SIZE * RUDP_BLOCKS_IN_SEQUENCE < curPacket->size(); ++i) {
                    std::shared_ptr<Packet> sequence(new Packet());

                    sequence->append(curPacket->data(),
                            std::min(curPacket->data().end(), std::next(curPacket->data().begin(), (i) * MAX_BLOCK_DATA_SIZE * RUDP_BLOCKS_IN_SEQUENCE)),
                            std::min(curPacket->data().end(), std::next(curPacket->data().begin(), (i+1) * MAX_BLOCK_DATA_SIZE * RUDP_BLOCKS_IN_SEQUENCE)));

                    _sequencesToSend.push(sequence);
                }

                sendNextSequence();
            }

            void Connection::onKeepAlive() {
                _engine->getNetworkEngine()->sendMessage(std::make_shared<KeepAliveMessage>(_endpConnId),
                        _node->address());
//                std::cout << "Kept him alive!!!\n";

                _pingJob = _engine->getSheduler()->executeTaskIn(std::chrono::milliseconds{PING_DELAY*1000},
                        std::bind(&Connection::onKeepAlive, this));
            }

        } //RUDP
    } //Network
} //p2p