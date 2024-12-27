#ifndef SSL_UTILS_HPP
#define SSL_UTILS_HPP

#include <openssl/ssl.h>

#include <iostream>

namespace ssl {

SSL_CTX *create_SSLctx();

}  // namespace ssl

#endif