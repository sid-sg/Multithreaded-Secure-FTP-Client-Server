#include <openssl/bio.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace bio {
BIO *create_socket_bio(const char *hostname, const char *port, int family);
}