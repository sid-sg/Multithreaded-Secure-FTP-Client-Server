#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
// #include <sys/sendfile.h>
#include <sys/socket.h>
// #include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
// #include <unistd.h>
#include <zlib.h>

#include <chrono>
// #include <cstring>
#include <iostream>
#include <regex>
#include <vector>

// Contains at least one lowercase letter, one uppercase letter, one digit, and is at least 8 characters long
const std::regex passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[!@#$%^&*(),.?\":{}|<>])[a-zA-Z\\d!@#$%^&*(),.?\":{}|<>]{8,}$");

// Contains only alphanumeric characters, underscores, and hyphens
const std::regex usernameRegex("^[a-zA-Z0-9._-]+$");

namespace handlers {
void serverHandler(SSL *ssl);
void registerUser(SSL *ssl);
bool checkLoggedIn(SSL *ssl);
void loginUser(SSL *ssl);
void listFiles(SSL *ssl);
void uploadFile(SSL *ssl);
void downloadFile(SSL *ssl);
void renameFile(SSL *ssl);
void deleteFile(SSL *ssl);

}  // namespace handlers

#endif
