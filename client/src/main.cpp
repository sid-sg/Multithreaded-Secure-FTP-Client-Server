#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "../include/bio_utils.hpp"
#include "../include/handlers.hpp"
#include "../include/ssl_utils.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

// int PORT;
char *PORT;
char *hostname;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_hostname> <server_portnum>"
                  << "\n";
        return 1;
    }

    hostname = argv[1];
    // PORT = atoi(argv[2]);
    PORT = argv[2];

    // int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // server fd
    // if (sockfd == -1) {
    //     std::cerr << "socket creation failed: " << std::strerror(errno) << "\n";
    //     return 1;
    // }
    // std::cout << "Socket created\n";

    // struct sockaddr_in serverAddr;

    // serverAddr.sin_family = AF_INET;
    // serverAddr.sin_port = htons(PORT);
    // serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    //     std::cerr << "failed connect(): " << std::strerror(errno) << "\n";
    // }
    // std::cout << "server connected\n";

    // handlers::serverHandler(sockfd);

    // close(sockfd);

    SSL_CTX *ctx = ssl::create_SSLctx();

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        std::cerr << "Failed to create the SSL object\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    BIO *bio = bio::create_socket_bio(hostname, PORT, AF_INET);
    if (bio == NULL) {
        std::cerr << "Failed to crete the BIO\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    SSL_set_bio(ssl, bio, bio);

    // Tell the server during the handshake which hostname we are attempting to connect to in case the server supports multiple hosts.
    if (!SSL_set_tlsext_host_name(ssl, hostname)) {
        std::cerr << "Failed to set the SNI hostname\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    // Ensure we check during certificate verification that the server has supplied a certificate for the hostname that we were expecting.
    if (!SSL_set1_host(ssl, hostname)) {
        std::cerr << "Failed to set the certificate verification hostname\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    /* Do the handshake with the server */
    if (SSL_connect(ssl) < 1) {
        std::cerr << "Failed to connect to the server\n";
        /*
         * If the failure is due to a verification error we can get more
         * information about it from SSL_get_verify_result().
         */
        if (SSL_get_verify_result(ssl) != X509_V_OK) printf("Verify error: %s\n", X509_verify_cert_error_string(SSL_get_verify_result(ssl)));
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    const char *request_start = "GET / HTTP/1.0\r\nConnection: close\r\nHost: ";
    const char *request_end = "\r\n\r\n";
    size_t written, readbytes;
    char buf[160];
    /* Write an HTTP GET request to the peer */
    if (!SSL_write_ex(ssl, request_start, strlen(request_start), &written)) {
        printf("Failed to write start of HTTP request\n");
        ERR_print_errors_fp(stderr);
    }
    if (!SSL_write_ex(ssl, hostname, strlen(hostname), &written)) {
        printf("Failed to write hostname in HTTP request\n");
        ERR_print_errors_fp(stderr);
    }
    if (!SSL_write_ex(ssl, request_end, strlen(request_end), &written)) {
        printf("Failed to write end of HTTP request\n");
        ERR_print_errors_fp(stderr);
    }

    while (SSL_read_ex(ssl, buf, sizeof(buf), &readbytes)) {
        fwrite(buf, 1, readbytes, stdout);
    }
    printf("\n");

    if (SSL_get_error(ssl, 0) != SSL_ERROR_ZERO_RETURN) {
        printf("Failed reading remaining data\n");
        ERR_print_errors_fp(stderr);
    }

    int ret = SSL_shutdown(ssl);
    if (ret < 1) {
        printf("Error shutting down\n");
        ERR_print_errors_fp(stderr);
    }

    return 0;
}