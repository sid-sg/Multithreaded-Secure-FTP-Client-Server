#ifndef HANDLERS_H
#define HANDLERS_H

#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

#include <cstring>
#include <iostream>
#include <regex>
#include <stdexcept>

const std::regex passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)[a-zA-Z\\d]{8,}$");
const std::regex usernameRegex("^[a-zA-Z0-9._-]+$");

class clientHandler {
    private:
        SSL* ssl;
        bool isLoggedin;
        std::string username;
        std::string folderdir;
    public:
        clientHandler(SSL* ssl) : ssl(ssl), isLoggedin(false) {};
        void handler();
        void registerUser();
        void sendLoggedInStatus();
        void loginUser();
        void listFiles();
        void uploadFile();
        void downloadFile();
        void renameFile();
        void deleteFile();
        
};

#endif
