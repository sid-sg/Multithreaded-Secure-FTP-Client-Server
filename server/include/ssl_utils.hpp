#ifndef SSL_UTILS_HPP
#define SSL_UTILS_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>


namespace ssl {

SSL_CTX *create_SSLctx(const char *cert_file, const char *key_file);

}  // namespace ssl
#endif