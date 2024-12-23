#include "../include/ssl_utils.hpp"

#include <openssl/bio.h>
#include <openssl/ssl.h>

namespace ssl {

SSL_CTX *create_SSLctx() {
    SSL_CTX *ctx = NULL;
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        std::cerr << "Unable to create SSL_CTX\n";
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    // if (!SSL_CTX_set_default_verify_paths(ctx)) {
    //     std::cerr << "Failed to set the default trusted certificate store\n";
    //     SSL_CTX_free(ctx);
    //     exit(EXIT_FAILURE);
    // }

    if (!SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION)) {
        std::cerr << "Failed to set the minimum TLS protocol version\n";
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

}  // namespace ssl
