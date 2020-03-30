#ifndef P2P_P2P_H
#define P2P_P2P_H

//#include "errors.h"
#include "p2p/config.h"
#include "p2pException.h"
#include "p2p/Network/NetworkEngine.h"
#include "p2p/DHT/DHT.h"
#include "p2p/Sheduler.h"
#include "p2p/Network/RUDP/Engine.h"
#include "p2p/Messenger/Account.h"

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace p2p {

	class P2P {
	public:
		P2P(uint16_t port = DEFAULT_UDP_PORT);
		~P2P();
		//if assertion fails or any other part of application wants to
		//emit an exception, it must call this method
		void handleError(const p2pException & err, const std::string & details = "");

		//endless loop uses this function to check whether the application is running or not application
		//can be stopped by calling void P2P::P2P::handleError(...) or void P2P::P2P::quit()
		bool isRunning();

		void sleep(uint32_t ms);

		void run();

		void printStats();
        void connect(boost::asio::ip::udp::endpoint endp);
		void findNode(const NodeId & nodeId, const DHT::NodeSearch::Callback & callback);
		void storeValue(const NodeId & nodeId, const NodeId & key, const DHT::StorageValueBuffer & val);
		void getValue(const NodeId & nodeId, const NodeId & key, const DHT::GetValueRequest::Callback & callback);
		void directConnection(const NodeId & nodeId, const std::string & key);
		void setRUDPMessageCallback(const std::string & key,
                                    const Network::RUDP::Connection::MessageCallback &callback);
		void sendMessage(const std::string &key, const std::shared_ptr<p2p::Network::RUDP::Packet> &packet);

		void addFriend(const std::string & key);
		void removeFriend(const std::string & key);
		uint64_t getTicks();

		//clean stopping
		void quit();

	private:
		void init();
		void update();
		void clean();

		uint64_t _ticks;

		struct _runable {
            explicit _runable (P2P & instance);
            void operator() ();
            P2P & _instance;
        };

        std::thread _runningThread;

        boost::asio::io_service _io;

        std::shared_ptr<Network::NetworkEngine> _networkEngine;
        std::shared_ptr<DHT::DHT> _dht;
        std::shared_ptr<Sheduler> _sheduler;
		std::shared_ptr<Network::RUDP::Engine> _rudpEngine;
		std::shared_ptr<Messenger::Account> _account;

		bool _running = false;

		uint16_t _udpPort;

	};
}

#endif
