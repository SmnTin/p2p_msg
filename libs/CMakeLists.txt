add_library(easy-ecc STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/easy-ecc/ecc.c)
add_library(hash-library STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-library/digest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-library/keccak.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-library/md5.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-library/sha1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-library/sha3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-library/sha256.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hash-library/crc32.cpp)
add_library(aes-library STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/AES/AES.cpp)