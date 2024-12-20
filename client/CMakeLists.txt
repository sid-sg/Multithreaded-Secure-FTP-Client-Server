cmake_minimum_required(VERSION 3.10)

project(FTP-client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/handlers.cpp
    src/progressbar.cpp
    src/ssl_utils.cpp
    src/bio_utils.cpp
)

add_executable(client ${SOURCES})

# Link libraries
find_package(ZLIB REQUIRED) 
find_package(OpenSSL REQUIRED)

target_link_libraries(client
    PRIVATE
    ZLIB::ZLIB
    OpenSSL::SSL
    OpenSSL::Crypto
)