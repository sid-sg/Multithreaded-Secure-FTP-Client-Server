#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>

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
        const char* client_hostname = BIO_get_conn_hostname(client_bio);
        const char* client_port = BIO_get_conn_port(client_bio);

        printf("New Client connected from hostname: %s, port: %s\n", client_hostname ? client_hostname : "unknown", client_port ? client_port : "unknown");

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

        unsigned char buf[8192];
        size_t nread;
        size_t nwritten;
        size_t total = 0;

        while (SSL_read_ex(ssl, buf, sizeof(buf), &nread) > 0) {
            if (SSL_write_ex(ssl, buf, nread, &nwritten) > 0 && nwritten == nread) {
                total += nwritten;
                continue;
            }
            std::cerr << ("Error echoing client input\n");
            break;
        }
        fprintf(stderr, "Client connection closed, %zu bytes sent\n", total);

        SSL_free(ssl);
    }

    SSL_CTX_free(ctx);
    return EXIT_SUCCESS;

    return 0;
}