#ifndef HANDLERS
#define HANDLERS

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <cstring>
#include <iostream>
#include <regex>
#include <stdexcept>

namespace handlers {
void clientHandler(SSL *ssl);
void listFiles(SSL *ssl, const std::string& clientname);
void uploadFile(SSL *ssl, const std::string& folderdir);
void downloadFile(SSL *ssl, const std::string& folderdir);
void renameFile(SSL *ssl, const std::string& folderdir);
void deleteFile(SSL *ssl, const std::string& folderdir);

// void clientHandler(SSL *ssl);
// void listFiles(SSL *ssl, const std::string& clientname);
// void uploadFile(SSL *ssl, const std::string& folderdir);
// void downloadFile(SSL *ssl, const std::string& folderdir);
// void renameFile(SSL *ssl, const std::string& folderdir);
// void deleteFile(SSL *ssl, const std::string& folderdir);
}  // namespace handlers

#endif
