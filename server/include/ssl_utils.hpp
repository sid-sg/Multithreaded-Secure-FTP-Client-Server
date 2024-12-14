#include <openssl/ssl.h>

#include <iostream>


namespace ssl {

void cleanup_SSL(SSL_CTX *ctx);

SSL_CTX *create_SSLctx(const char *cert_file, const char *key_file);

}  // namespace ssl