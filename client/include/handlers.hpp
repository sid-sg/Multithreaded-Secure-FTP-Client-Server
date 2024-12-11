#ifndef HANDLERS
#define HANDLERS

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

namespace handlers {
void serverHandler(int clientfd);
void listFiles(int clientfd);
void uploadFile(int clientfd);
void downloadFile(int clientfd);
void renameFile(int clientfd);
void deleteFile(int clientfd);
}  // namespace handlers

#endif
