cmake_minimum_required(VERSION 2.8)
project(p2p_msg)

set(CMAKE_CXX_STANDARD 17)
find_package (Threads)

include_directories (
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/libs"
)

set(SOURCE src/p2p/NodeId.cpp src/p2p/P2P.cpp src/p2p/p2pAssert.cpp src/p2p/Network/NetworkEngine.cpp src/p2p/DHT/RoutingTable.cpp src/p2p/DHT/DHTMessagehandlers.cpp src/p2p/Messenger/Account.cpp src/p2p/Network/RUDP/Connection.cpp src/p2p/Network/RUDP/RUDPMessageHandlers.cpp src/p2p/DHT/DHT.cpp src/p2p/DHT/Searcher.cpp src/p2p/Sheduler.cpp)

add_executable(p2p_msg ${SOURCE} src/main.cpp)
target_link_libraries(p2p_msg ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} cryptopp-shared)