#ifndef UTILS
#define UTILS

#include <dirent.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace utils {
bool isRoot();
std::string ls(std::string directory);
bool ensureDirectory(const std::string& path);
off_t getFilesize(int filefd);
void sendError(int clientfd, const std::string& errorMsg);
}  // namespace utils

#endif