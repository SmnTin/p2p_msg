cmake_minimum_required(VERSION 2.8)
project(p2p_msg)

set(CMAKE_CXX_STANDARD 17)
find_package (Threads)
find_package(Boost COMPONENTS system filesystem random REQUIRED)

include_directories (
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/libs"
        "${Boost_INCLUDE_DIR}"
)

add_subdirectory(libs)

set(SOURCE src/p2p/NodeId.cpp src/p2p/P2P.cpp src/p2p/Network/NetworkEngine.cpp src/p2p/DHT/RoutingTable.cpp src/p2p/DHT/DHTMessagehandlers.cpp src/p2p/Network/RUDP/Connection.cpp src/p2p/Network/RUDP/RUDPMessageHandlers.cpp src/p2p/DHT/DHT.cpp src/p2p/DHT/Searcher.cpp src/p2p/Sheduler.cpp src/p2p/Crypto/ECC.cpp src/p2p/Crypto/Hash.cpp src/p2p/Crypto/AES.cpp)

add_library(p2p_msg STATIC ${SOURCE})
target_link_libraries(p2p_msg ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} easy-ecc hash-library aes-library)

add_executable(p2p_exec ${SOURCE} src/main.cpp)
target_link_libraries(p2p_exec p2p_msg)

add_executable(p2p_test src/test_main.cpp)
target_link_libraries(p2p_test p2p_msg)