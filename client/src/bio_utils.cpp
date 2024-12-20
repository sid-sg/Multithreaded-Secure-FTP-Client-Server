#include "../include/bio_utils.hpp"

namespace bio {
BIO *create_socket_bio(const char *hostname, const char *port, int family) {
    BIO_ADDRINFO *res = NULL;

    // Lookup IP address info for the server
    if (!BIO_lookup_ex(hostname, port, BIO_LOOKUP_CLIENT, family, SOCK_STREAM, 0, &res)) return NULL;

    int sock = -1;
    const BIO_ADDRINFO *ai = NULL;

    for (ai = res; ai != NULL; ai = BIO_ADDRINFO_next(ai)) {
        sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, 0, 0);
        if (sock == -1) continue;

        /* Connect the socket to the server's address */
        if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY)) {
            ERR_print_errors_fp(stderr);
            BIO_closesocket(sock);
            sock = -1;
            continue;
        }

        /* We have a connected socket so break out of the loop */
        break;
    }

    BIO_ADDRINFO_free(res);

    if (sock == -1) return NULL;

    /* Create a BIO to wrap the socket */
    BIO *bio = BIO_new(BIO_s_socket());
    if (bio == NULL) {
        ERR_print_errors_fp(stderr);
        BIO_closesocket(sock);
        return NULL;
    }

    // BIO_CLOSE ensures the socket will be automatically closed when the BIO is freed
    BIO_set_fd(bio, sock, BIO_CLOSE);

    return bio;
}

}  // namespace bio