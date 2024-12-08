#include "utils.hpp"

namespace utils {

bool isRoot() {
    if (getuid() != 0) {
        return false;
    } else {
        return true;
    }
}

std::string ls(std::string directory) {
    DIR* d = opendir(directory.c_str());
    struct dirent* dir;
    std::string files;

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                std::string filename = dir->d_name;
                if ((filename.size() > 3) && (filename.substr(filename.size() - 3) == ".gz")) {
                    filename = filename.substr(0, filename.size() - 3);
                }
                files += filename;
                files += "\n";
            }
        }
        closedir(d);
    }

    return files;
}

bool ensureDirectory(const std::string& path) {
    struct stat st;
    memset(&st, 0, sizeof(st));

    if (stat(path.c_str(), &st) == -1) {
        if (mkdir(path.c_str(), 0700) == -1) {
            std::cerr << "Failed to create directory " << path << ": " << strerror(errno) << "\n";
            return false;
        }
    }
    return true;
}

off_t getFilesize(int filefd) {
    off_t filesize = lseek(filefd, 0, SEEK_END);
    lseek(filefd, 0, SEEK_SET);
    return filesize;
}

void sendError(int clientfd, const std::string& errorMsg) { send(clientfd, errorMsg.c_str(), errorMsg.length(), 0); }

}  // namespace utils