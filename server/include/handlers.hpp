#ifndef HANDLERS
#define HANDLERS

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

#include <cstring>
#include <iostream>

namespace handlers {
void clientHandler(int clientfd);
void listFiles(int clientfd, const std::string& clientname);
void uploadFile(int clientfd, const std::string& folderdir);
void downloadFile(int clientfd, const std::string& folderdir);
void renameFile(int clientfd, const std::string& folderdir);
void deleteFile(int clientfd, const std::string& folderdir);
}  // namespace handlers

#endif
