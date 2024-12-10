cmake_minimum_required(VERSION 3.10)

project(FTP-server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/handlers.cpp
    src/utils.cpp
    src/threadpool.cpp
)

add_executable(server ${SOURCES})

# Link libraries
find_package(ZLIB REQUIRED) 
target_link_libraries(server PRIVATE ZLIB::ZLIB pthread)
