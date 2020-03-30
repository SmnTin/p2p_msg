cmake_minimum_required(VERSION 2.8)
project(p2p_msg)

set(CMAKE_CXX_STANDARD 14)
find_package(Boost COMPONENTS system filesystem random REQUIRED)
find_package (Threads)
find_package (CryptoPP REQUIRED)

include_directories (
        "${PROJECT_SOURCE_DIR}/include"
        "${Boost_INCLUDE_DIR}"
)


set(SOURCE src/p2p/NodeId.cpp src/p2p/P2P.cpp src/p2p/p2pAssert.cpp include/p2p/Network/Message.h include/p2p/DHT/RoutingTable.h include/p2p/Network/NetworkEngine.h src/p2p/Network/NetworkEngine.cpp include/p2p/DHT/DHTMessageHandlers.h src/p2p/DHT/RoutingTable.cpp include/p2p/Network/MessageHandler.h src/p2p/DHT/DHTMessagehandlers.cpp include/p2p/DHT/DHTMessages.h include/p2p/Node.h include/p2p/Sheduler.h include/p2p/Messenger/Account.h src/p2p/Messenger/Account.cpp include/p2p/Network/RUDP/Engine.h include/p2p/Network/RUDP/RUDPMessageHandlers.h include/p2p/Network/RUDP/RUDPMessages.h include/p2p/Network/RUDP/Packet.h include/p2p/Network/RUDP/Connection.h src/p2p/Network/RUDP/Connection.cpp src/p2p/Network/RUDP/RUDPMessageHandlers.cpp include/p2p/Messenger/Friend.h include/p2p/Messenger/Message.h include/p2p/OstreamMethods.h include/p2p/DHT/DHT.h include/p2p/DHT/Searcher.h src/p2p/DHT/DHT.cpp src/p2p/DHT/Searcher.cpp src/p2p/Sheduler.cpp)

add_executable(p2p_msg ${SOURCE} src/main.cpp)
target_link_libraries(p2p_msg ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} cryptopp-shared)