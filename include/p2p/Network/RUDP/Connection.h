#ifndef P2P_MSG_CONNECTION_H
#define P2P_MSG_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "p2p/Node.h"
#include "p2p/Network/RUDP/Packet.h"
#include "p2p/Sheduler.h"

#include <queue>
#include <bitset>
#include <array>
#include <vector>
#include <chrono>
#include <mutex>
#include <functional>

namespace p2p {
    namespace Network {
        namespace RUDP {

            class Engine;

            typedef uint16_t BlockIndex;
            typedef uint16_t ConnectionId;
            constexpr size_t MAX_CONNECTIONS = ((size_t)1 << (sizeof(ConnectionId) * 8));

            class Connection {
            public:
                Connection();
                Connection(Engine * engine, ConnectionId connId, std::shared_ptr<Node> node);
                ~Connection();

                void connect();
                void acceptConnection(ConnectionId endpConnId);

                void send(const std::shared_ptr<Packet> & packet);

                void onDisconnection();
                void onResponseMessage();
                void onEachMessage(boost::asio::ip::udp::endpoint &endp);
                void onConnectionResponse();
                void onSequenceEndRequest(bool terminal = false);
                void onSequenceEndResponse();
                void onBlock(const BlockIndex & ind, const MessageBuffer & val, const MessageLength & valLen);
                void onAckRequest();
                void onAckResponse(const std::bitset<RUDP_BLOCKS_IN_SEQUENCE> & receivedBlocks);

                typedef std::function<void(std::shared_ptr<Packet>)> MessageCallback;
                typedef std::function<void()> ConnectionCallback;
                typedef std::function<void()> DisconnectionCallback;

                void setMessageCallback(const MessageCallback & callback);
                void setConnectionCallback(const ConnectionCallback & callback);
                void setDisconnectionCallback(const DisconnectionCallback & callback);

                void disconnect();

                std::shared_ptr<Node> getNode();
                ConnectionId getConnectionId();
                ConnectionId getEndpointConnectionId();

            private:
                std::shared_ptr<Node> _node;
                ConnectionId _connId;
                ConnectionId _endpConnId;
                Engine * _engine;

                MessageCallback onMessageCallback;
                ConnectionCallback onConnectionCallback;
                DisconnectionCallback onDisconnectionCallback;

                bool _anySent = false;
                std::chrono::steady_clock::time_point _lastSent;
                std::chrono::milliseconds _RTT {200};

                void _onConnectionTimeout();

                void makeAttempts(std::function<void()> onEvery, std::function<void()> onFail, int maxAttempts = -1, int attempts = 1);
                void stopAttempts();

                void sendPacket();
                void sendNextSequence();
                void sendSequence();

                void onKeepAlive();

                std::queue<std::shared_ptr<Packet>> _packetsToSend;
                bool _isSending = false;

                std::queue<std::shared_ptr<Packet>> _sequencesToSend;
                std::bitset<RUDP_BLOCKS_IN_SEQUENCE> _sentBlocks;
                size_t _blocksInSequence = 0;
                std::array<std::shared_ptr<Packet>, RUDP_BLOCKS_IN_SEQUENCE> _sequenceToSend;
                std::recursive_mutex _sendMutex;

                std::bitset<RUDP_BLOCKS_IN_SEQUENCE> _receivedBlocks;
                std::array<std::shared_ptr<Packet>, RUDP_BLOCKS_IN_SEQUENCE> _sequenceReceived;
                std::queue<std::shared_ptr<Packet>> _sequencesReceived;
                std::recursive_mutex _receiveMutex;

                std::shared_ptr<Task> _responseJob;
                std::shared_ptr<Task> _pingJob;
                std::shared_ptr<Task> _connectionTimeoutJob;
            };
        } //RUDP
    } //Network
} //p2p

#endif //P2P_MSG_CONNECTION_H
