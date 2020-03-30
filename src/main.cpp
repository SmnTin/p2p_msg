#include <iostream>
#include <thread>
#include <string>

#include "p2p/P2P.h"

using namespace std;
using p2p::Network::RUDP::Packet;

class findNode {
public:
    void fuck(std::shared_ptr<p2p::Node> node) {
        if(node == nullptr) {
            std::cout << "Node not found.\n";
            return;
        }
        std::cout << "FOUND NODE !!! \n" << node->id() << "\n";
    }
};

int main() {
    cout << "Welcome to p2p-msg core!" << endl;

    std::shared_ptr<p2p::P2P> p2pInstance(nullptr);

    while(true) {
//        cout << "\n>>>";
        string comm;
        cin >> comm;
        if(comm == "start") {
            if(p2pInstance != nullptr)
                continue;
            uint16_t port;
            cin >> port;
            p2pInstance = std::make_shared<p2p::P2P>(port);
            p2pInstance->run();
        } else if(comm == "stat") {
            if(p2pInstance == nullptr)
                continue;
            p2pInstance->printStats();
        } else if(comm == "stop") {
            if(p2pInstance == nullptr)
                continue;
            p2pInstance->quit();
            break;
        } else if(comm == "connect") {
            if(p2pInstance == nullptr)
                continue;

            string addr;
            uint16_t port;
            cin >> addr >> port;

            p2pInstance->connect(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(addr), port));
        } else if(comm == "find") {
            if(p2pInstance == nullptr)
                continue;

            p2p::NodeId nodeId;
            cin >> nodeId;
            cout << nodeId << "\n";

            p2p::DHT::NodeSearch::Callback lambda = [](std::shared_ptr<p2p::Node> node) {
                if(node == nullptr) {
                    std::cout << "Node not found.\n";
                    return;
                }
                std::cout << "FOUND NODE !!! \n" << node->id() << "\n";
            };

            findNode * f = new findNode();
            p2p::DHT::NodeSearch::Callback callback = std::bind(&findNode::fuck, f, std::placeholders::_1);
            p2pInstance->findNode(nodeId, callback);
        } else if(comm == "store") {
            if(p2pInstance == nullptr)
                continue;

            p2p::NodeId nodeId;
            cin >> nodeId;

            p2p::NodeId key;
            cin >> key;

            std::string str;
            cin >> str;
            p2p::DHT::StorageValueBuffer val;
            for(int i = 0; i < std::min(str.size(), (size_t)p2p::DHT::MAX_STORAGE_VALUE_BUFFER_SIZE); ++i)
                val[i] = (uint8_t)str[i];

            p2pInstance->storeValue(nodeId, key, val);
        } else if(comm == "get") {
            if(p2pInstance == nullptr)
                continue;

            p2p::NodeId nodeId;
            cin >> nodeId;

            p2p::NodeId key;
            cin >> key;


            p2p::DHT::GetValueRequest::Callback lambda = [](bool success, p2p::DHT::StorageValueBuffer val) {
                if(!success) {
                    std::cout << "Value not found.\n";
                    return 0;
                }
                std::cout << "Found value!\n";
                std::string str;
                for(int i = 0; i < (size_t)p2p::DHT::MAX_STORAGE_VALUE_BUFFER_SIZE; ++i)
                    str += (unsigned char)val[i];
                std::cout << str << "\n";
            };

            p2pInstance->getValue(nodeId, key, lambda);
        } else if(comm == "add") {
            if(p2pInstance == nullptr)
                continue;

            std::string key;
            cin >> key;

            p2pInstance->addFriend(key);
        } else if(comm == "direct") {
            p2p::NodeId nodeId;
            cin >> nodeId;
            std::string key;
            cin >> key;

            p2pInstance->directConnection(nodeId, key);
            p2p::Network::RUDP::Connection::MessageCallback msgCB = [](std::shared_ptr<Packet> packet) {
                std::cout << "Received \"packet\":\n";
                for(char c : packet->data())
                    std::cout << c;
                std::cout << "\n";
            };
            p2pInstance->setRUDPMessageCallback(key, msgCB);
        } else if(comm == "send") {
            std::string key;
            cin >> key;
            std::string msg;
            cin >> msg;
            std::vector<uint8_t> msgData (msg.size());
            std::copy(msg.begin(), msg.end(), msgData.begin());
            std::shared_ptr<Packet> packet (new Packet());
            packet->append(msgData);

            p2pInstance->sendMessage(key, packet);
        }
//        p2pInstance.sleep(1000);
    }

    return 0;
}
