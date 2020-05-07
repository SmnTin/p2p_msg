add_library(easy-ecc STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/easy-ecc/ecc.c)
add_library(hash-lib STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-lib/digest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-lib/keccak.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-lib/md5.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-lib/sha1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-lib/sha3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-lib/sha256.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-lib/crc32.cpp)
add_library(aes-library STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/AES/AES.cpp)