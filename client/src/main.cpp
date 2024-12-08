#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "handlers.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

int PORT;
char *SERVER_IP;


int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip_address> <server_portnum>"
                  << "\n";
        return 1;
    }

    SERVER_IP = argv[1];
    PORT = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // server fd
    if (sockfd == -1) {
        std::cerr << "socket creation failed: " << std::strerror(errno) << "\n";
        return 1;
    }
    std::cout << "Socket created\n";

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "failed connect(): " << std::strerror(errno) << "\n";
    }
    std::cout << "server connected\n";

    // char buffer[BUFFER_SIZE];
    handlers::serverHandler(sockfd);

    close(sockfd);

    return 0;
}