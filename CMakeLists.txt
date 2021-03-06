cmake_minimum_required(VERSION 3.2)
project(p2p_msg VERSION 0.0.0.0)

#------------------------FLAGS-------------------------------#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${PROJECT_SOURCE_DIR}/,,$(abspath $<))\"'")

#--------------------LIBRARIES & INCLUDE---------------------#
find_package(Threads)
find_package(Boost COMPONENTS system thread REQUIRED)

include_directories(
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/libs"
        "${Boost_INCLUDE_DIR}"
)

add_subdirectory(libs)

#-------------------------GTEST-------------------------------#

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
    include_directories("${gmock_SOURCE_DIR}/include")
endif ()

#-----------------------SOURCE---------------------------#
file(GLOB_RECURSE HEADERS
        "include/*.h")

file(GLOB_RECURSE SOURCE
        "src/*.h"
        "src/*.cpp")

#-----------------------TARGETS--------------------------#
add_library(p2p_msg STATIC ${HEADERS} ${SOURCE})
target_link_libraries(p2p_msg ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES}
        easy-ecc hash-library aes-library function2 continuable)

add_executable(p2p_exec ${SOURCE})
target_link_libraries(p2p_exec p2p_msg)

#-----------------------TESTS----------------------------#
file(GLOB_RECURSE TESTS
        "test/*.h"
        "test/*.cpp"
        )
add_executable(test ${TESTS})

target_include_directories(test PRIVATE
        "${PROJECT_SOURCE_DIR}/test"
        "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(test
        gtest_main
        gmock_main
        p2p_msg
        )

add_custom_target(run
        COMMAND test
        DEPENDS test
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
        )