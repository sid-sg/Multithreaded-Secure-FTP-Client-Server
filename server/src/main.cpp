#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/bio.h>
#include <signal.h>
#include <sys/socket.h>
// #include <sys/stat.h>
#include <sys/types.h>

// #include <cerrno>
// #include <cstring>
#include <thread>

#include "../include/db.hpp"
#include "../include/file_utils.hpp"
#include "../include/handlers.hpp"
#include "../include/ssl_utils.hpp"
#include "../include/threadpool.hpp"

#define BACKLOG 20
#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

// int PORT;
const char* PORT;

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    // if (!utils::isRoot()) {
    //     std::cerr << "This program must be run as root/sudo user\n";
    //     return 1;
    // }

    SSL_CTX* ctx = ssl::create_SSLctx("../security/server.crt", "../security/server.key");

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <portnum>"
                  << "\n";
        return 1;
    }

    // PORT = atoi(argv[1]);
    PORT = argv[1];

    if (!utils::ensureDirectory("../../store")) {
        std::cerr << "Failed to create store directory\n";
        return 1;
    }

    Database db("../data/user.db");
    if (!db.initTable()) {
        std::cerr << "Failed to initialize database table\n";
        return 1;
    }

    std::cout << "User table initialized\n";

    /*
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
    */

    BIO* acceptor_bio = BIO_new_accept(PORT);  // BIO_new() + BIO_set_accept_name() / socket()
    if (!acceptor_bio) {
        std::cerr << "Error creating acceptor BIO\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    // int yes = 1;

    // if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {  // to avoid "port already in use" error
    //     std::cerr << "setsockopt() failed: " << std::strerror(errno) << "\n";
    //     close(sockfd);
    //     return 1;
    // }

    // if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
    //     std::cerr << "failed binding to port: " << PORT << " " << std::strerror(errno) << "\n";
    //     close(sockfd);
    //     return 1;
    // }

    // if (listen(sockfd, BACKLOG) == -1) {
    //     std::cerr << "failed listening: " << std::strerror(errno) << "\n";
    //     close(sockfd);
    //     return 1;
    // }

    BIO_set_bind_mode(acceptor_bio, BIO_BIND_REUSEADDR);  // setsockopt() + bind() + listen()
    if (BIO_do_accept(acceptor_bio) <= 0) {
        std::cerr << "Error setting up acceptor socket";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    int server_fd;
    BIO_get_fd(acceptor_bio, &server_fd);
    int nodelay_flag = 1;

    if (setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay_flag, sizeof(nodelay_flag)) == -1) {
        std::cerr << "Failed to set TCP_NODELAY: " << std::strerror(errno) << "\n";
    }

    std::cout<<"Disabled Nagle's algorithm\n";

    std::cout << "Server listening on port: " << PORT << "\n";

    // struct sockaddr_in clientAddr;
    // socklen_t clientAddrSize = sizeof(clientAddr);

    Threadpool pool(16);

    while (1) {
        BIO* client_bio = NULL;
        SSL* ssl = NULL;
        // int clientfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);  // client fd

        // if (clientfd == -1) {
        //     std::cerr << "failed accept(): " << std::strerror(errno) << "\n";
        //     continue;
        // }
        // std::cout << "client connected: Address=" << inet_ntoa(clientAddr.sin_addr) << " Port=" << ntohs(clientAddr.sin_port) << "\n";
        // pool.enqueueTask(handlers::clientHandler, clientfd);

        if (BIO_do_accept(acceptor_bio) <= 0) {  // Wait for the next client to connect
            std::cerr << "Error accepting client connection\n";
            ERR_print_errors_fp(stderr);
            continue;
        }

        client_bio = BIO_pop(acceptor_bio);  // Pop the client connection from the BIO chain
        if (!client_bio) {
            std::cerr << "Error popping client BIO\n";
            ERR_print_errors_fp(stderr);
            continue;
        }
        // get client address
        struct sockaddr_storage addr;
        socklen_t addr_len = sizeof(addr);
        int client_fd;

        BIO_get_fd(client_bio, &client_fd);
        getpeername(client_fd, (struct sockaddr*)&addr, &addr_len);

        char client_ip[INET6_ADDRSTRLEN];
        int client_port;

        if (addr.ss_family == AF_INET) {
            struct sockaddr_in* s = (struct sockaddr_in*)&addr;
            inet_ntop(AF_INET, &s->sin_addr, client_ip, sizeof(client_ip));
            client_port = ntohs(s->sin_port);
        } else {
            struct sockaddr_in6* s = (struct sockaddr_in6*)&addr;
            inet_ntop(AF_INET6, &s->sin6_addr, client_ip, sizeof(client_ip));
            client_port = ntohs(s->sin6_port);
        }

        printf("New Client connected from IP: %s, Port: %d\n", client_ip, client_port);

        if ((ssl = SSL_new(ctx)) == NULL) {  // Associate a new SSL handle with the new connection
            std::cerr << "Error creating SSL handle for new connection\n";
            ERR_print_errors_fp(stderr);
            BIO_free(client_bio);
            continue;
        }
        SSL_set_bio(ssl, client_bio, client_bio);

        /* Attempt an SSL handshake with the client */
        if (SSL_accept(ssl) <= 0) {
            std::cerr << "Error performing SSL handshake with client\n";
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            continue;
        }

        printf("SSL handshake success with client from IP: %s, Port: %d\n", client_ip, client_port);

        pool.enqueueTask([ssl, client_ip, client_port]() {
            try {
                clientHandler handler(ssl, client_ip, client_port);
                handler.handler();
            } catch (const std::exception& e) {
                std::cerr << "Exception in client handler: " << e.what() << "\n";
                SSL_shutdown(ssl);
                SSL_free(ssl);
            } catch (...) {
                std::cerr << "Unknown error occurred in client handler.\n";
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
        });
        // Handle client in new thread
        // std::thread(handle_client, ssl).detach();
    }

    BIO_free_all(acceptor_bio);
    SSL_CTX_free(ctx);

    return 0;
}