#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>

#include "../include/handlers.hpp"
#include "../include/threadpool.hpp"
#include "../include/utils.hpp"

#define BACKLOG 20
#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

int PORT;

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);
    // if (!utils::isRoot()) {
    //     std::cerr << "This program must be run as root/sudo user\n";
    //     return 1;
    // }

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <portnum>"
                  << "\n";
        return 1;
    }

    PORT = atoi(argv[1]);

    if (!utils::ensureDirectory("../../store")) {
        std::cerr << "Failed to create store directory\n";
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // server fd
    if (sockfd == -1) {
        std::cerr << "socket creation failed: " << std::strerror(errno) << "\n";
        return 1;
    }
    std::cout << "Socket created\n";

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int yes = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {  // to avoid "port already in use" error
        std::cerr << "setsockopt() failed: " << std::strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "failed binding to port: " << PORT << " " << std::strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        std::cerr << "failed listening: " << std::strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    std::cout << "Server listening on port: " << PORT << "\n";

    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    Threadpool pool(16);

    while (1) {
        int clientfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);  // client fd

        if (clientfd == -1) {
            std::cerr << "failed accept(): " << std::strerror(errno) << "\n";
            continue;
        }

        std::cout << "client connected: Address=" << inet_ntoa(clientAddr.sin_addr) << " Port=" << ntohs(clientAddr.sin_port) << "\n";

        pool.enqueueTask(handlers::clientHandler, clientfd);
    }

    close(sockfd);

    return 0;
}