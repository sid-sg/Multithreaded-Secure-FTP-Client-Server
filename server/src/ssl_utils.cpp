#include "../include/ssl_utils.hpp"

static const char *CACHE_ID = "OpenSSL Demo Server";

namespace ssl {

SSL_CTX *create_SSLctx(const char *cert_file, const char *key_file) {
    SSL_CTX *ctx = NULL;
    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        std::cerr << "Unable to create SSL_CTX\n";
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    if (!SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION)) {
        std::cerr << "Failed to set the minimum TLS protocol version\n";
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_certificate_chain_file(ctx, cert_file) <= 0) {
        std::cerr << "Failed to load the server certificate chain file\n";
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Error loading server private key file\n";
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_session_id_context(ctx, reinterpret_cast<const unsigned char *>(CACHE_ID), sizeof(CACHE_ID));
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);  // enables server-side session caching

    SSL_CTX_sess_set_cache_size(ctx, 1024);  // how many client TLS sessions to cache

    SSL_CTX_set_timeout(ctx, 3600);  // sessions older than this are considered a cache miss even if still in the cache

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);  // client cert auth not needed

    return ctx;
}

}  // namespace ssl
