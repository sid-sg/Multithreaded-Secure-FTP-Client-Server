#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
// #include <sys/sendfile.h>
// #include <sys/stat.h>
// #include <sys/types.h>
#include <zlib.h>

// #include <cstring>
#include <iostream>
#include <mutex>
#include <regex>
#include <unordered_map>
// #include <stdexcept>

// Contains at least one lowercase letter, one uppercase letter, one digit, and is at least 8 characters long
const std::regex passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[!@#$%^&*(),.?\":{}|<>])[a-zA-Z\\d!@#$%^&*(),.?\":{}|<>]{8,}$");

// Contains only alphanumeric characters, underscores, and hyphens
const std::regex usernameRegex("^[a-zA-Z0-9._-]+$");

class clientHandler {
   private:
    SSL* ssl;
    bool isLoggedin;
    std::string username;
    std::string folderdir;
    std::string client_ip;
    int client_port;
    static std::unordered_map<std::string, std::mutex> folderLocks;

    void clientInfo();
    void sendLoggedInStatus();

   public:
    clientHandler(SSL* ssl, const std::string client_ip, int client_port) : ssl(ssl), client_ip(client_ip), client_port(client_port), isLoggedin(false){};
    void handler();
    void registerUser();
    void loginUser();
    void listFiles();
    void uploadFile();
    void downloadFile();
    void renameFile();
    void deleteFile();
};

#endif
