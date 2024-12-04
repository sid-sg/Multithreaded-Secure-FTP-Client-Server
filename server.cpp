#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "./include/threadpool.hpp"

#define PORT 7000
#define BACKLOG 20
#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

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

void clientHandler(int clientfd) {
    char buffer[BUFFER_SIZE];

    // recieve name
    int bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);

    if (bytesRecv <= 0) {
        std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
        return;
    }

    std::string clientname(buffer, bytesRecv);

    std::cout << "Received from client: \n";
    std::cout << "Name: " << clientname << "\n";
    std::string folderdir = "./store/" + clientname;

    struct stat st;
    memset(&st, 0, sizeof(st));

    if (stat(folderdir.c_str(), &st) == -1) {
        mkdir(folderdir.c_str(), 0700);
    }
    memset(buffer, 0, sizeof(buffer));

    while (1) {
        // recieve option
        bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);

        if (bytesRecv <= 0) {
            std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
            break;
        }

        std::string option(buffer, bytesRecv);
        std::cout << "Option selected: " << option << "\n";

        memset(buffer, 0, sizeof(buffer));

        if (option == "ls") {
            std::string directory = std::string("./store/") + clientname;
            std::string files = ls(directory);

            if (files.empty()) {
                files = "Empty directory";
            }

    memset(buffer, 0, sizeof(buffer));
            if (files.size() < BUFFER_SIZE) {
                strcpy(buffer, files.c_str());
            } else {
                std::cerr << "Directory files is too large for buffer\n";
                close(clientfd);
                continue;
            }

            std::cout << "ls content: " << buffer;

            // Send the directory files to the client
            int bytesSent = send(clientfd, buffer, strlen(buffer), 0);
            if (bytesSent == -1) {
                std::cerr << "Failed sending directory files: " << std::strerror(errno) << "\n";
                break;
            }
            std::cout << "Sent directory files to client\n";
        } else if (option == "upload") {

    memset(buffer, 0, sizeof(buffer));
            bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv file metadata (filename:filesize)
            if (bytesRecv <= 0) {
                std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
                break;
            }

            std::string metadata(buffer, bytesRecv);

            int partition = metadata.find(":");
            std::string filename = metadata.substr(0, partition);
            int filesize = std::stoi(metadata.substr(partition + 1));

            std::cout << "Filename: " << filename << "\n";
            std::cout << "Filesize: " << filesize << "\n";

            int bytesSent = send(clientfd, "OK", 2, 0);
            if (bytesSent == -1) {
                std::cerr << "Failed sending ACK: " << std::strerror(errno) << "\n";
                break;
            }

            // std::string filepath = folderdir + "/" + filename;

            std::string compressedFilepath = folderdir + "/" + filename + ".gz";

            // int filefd = open(compressedFilepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            // if (filefd == -1) {
            //     std::cerr << "Failed creating file: " << std::strerror(errno) << "\n";
            //     break;
            // }

            gzFile compressedfile = gzopen(compressedFilepath.c_str(), "wb");
            if (!compressedfile) {
                std::cerr << "Failed creating compressed file: " << std::strerror(errno) << "\n";
                break;
            }

    memset(buffer, 0, sizeof(buffer));
            int remaining = filesize;
            while (remaining > 0) {
                bytesRecv = recv(clientfd, buffer, std::min(remaining, BUFFER_SIZE), 0);
                if (bytesRecv <= 0) {
                    std::cerr << "Client disconnected while transferring file: " << std::strerror(errno) << "\n";
                    gzclose(compressedfile);
                    break;
                }
                gzwrite(compressedfile, buffer, bytesRecv);
                remaining -= bytesRecv;
            }

            std::cout << "File received and saved to " << compressedFilepath << "\n";
            gzclose(compressedfile);
        } else if (option == "download") {

    memset(buffer, 0, sizeof(buffer));
            bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv filename to download
            if (bytesRecv <= 0) {
                std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
                break;
            }

            std::string filename(buffer, bytesRecv);
            filename += ".gz";
            memset(buffer, 0, sizeof(buffer));

            std::string filepath = folderdir + "/" + filename;

            int filefd = open(filepath.c_str(), O_RDONLY);
            if (filefd == -1) {
                std::cerr << "File not found: " << filename << "\n";
                std::string errorMsg = "ERROR: File Not Found";
                send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
                return;
            }

            off_t filesize = lseek(filefd, 0, SEEK_END);
            lseek(filefd, 0, SEEK_SET);

            std::string data = std::to_string(filesize);
            int bytesSent = send(clientfd, data.c_str(), data.length(), 0);
            if (bytesSent == -1) {
                std::cerr << "Failed sending compressed filesize: " << std::strerror(errno) << "\n";
                continue;
            }

            char ACK[3];

            int bytesRecv = recv(clientfd, ACK, sizeof(ACK) - 1, 0);

            ACK[bytesRecv] = '\0';

            if (bytesRecv == -1) {
                std::cerr << "failed recving ACK: " << std::strerror(errno) << "\n";
                continue;
            }

            std::cout << "ACK: " << ACK << "\n";

            if (strcmp(ACK, "OK") != 0) {
                std::cerr << "recieved NACK: "
                          << "\n";
                continue;
            }

            off_t totalBytesSent = 0;
            off_t bytesRemaining = filesize;
            const size_t chunkSize = 65536;

            while (bytesRemaining > 0) {
                size_t remaining = std::min(chunkSize, static_cast<size_t>(bytesRemaining));
                bytesSent = sendfile(clientfd, filefd, &totalBytesSent, remaining);
                if (bytesSent == -1) {
                    std::cerr << "failed sending file: " << std::strerror(errno) << "\n";
                    close(filefd);
                    break;
                }

                bytesRemaining -= bytesSent;
            }

            std::cout << "Compressed file sent to client\n";

        } else if (option == "rename") {
            bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv file metadata
            if (bytesRecv <= 0) {
                std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
                break;
            }

            std::string metadata(buffer, bytesRecv);
            memset(buffer, 0, sizeof(buffer));

            std::string::size_type partition = metadata.find(":");  // Example usage
            if (partition == std::string::npos) {
                std::cerr << "Invalid metadata received: " << metadata << "\n";
                std::string errorMsg = "ERROR: Invalid metadata format";
                send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
                continue;
            }

            std::string filename = metadata.substr(0, partition);
            std::string newname = metadata.substr(partition + 1);

            std::string filepath = folderdir + "/" + filename + ".gz";
            std::string newpath = folderdir + "/" + newname + ".gz";

            std::cout << "filepath: " << filepath << "\n";
            std::cout << "newpath: " << newpath << "\n";

            // struct stat st = {0};
            // int filefd = open(filepath.c_str(), O_RDONLY);
            // if ( filefd == -1) {
            //     std::cerr << "File not found: " << filename << "\n";
            //     std::string errorMsg = "ERROR: File Not Found";
            //     close(filefd);
            //     send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
            // }
            // close(filefd);

            if (rename(filepath.c_str(), newpath.c_str()) != 0) {
                std::cerr << "Failed to rename file: " << std::strerror(errno) << "\n";
                std::string errorMsg = "ERROR: Failed to rename file";
                send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
                continue;
            }

            strcpy(buffer, "success");
            int bytesSent = send(clientfd, "", sizeof(buffer), 0);
            if (bytesSent == -1) {
                std::cerr << "failed sending message: " << std::strerror(errno) << "\n";
                continue;
            }

        } else if (option == "delete") {
            bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv filename
            if (bytesRecv <= 0) {
                std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
                break;
            }

            std::string filename(buffer, bytesRecv);
            std::string filepath = folderdir + "/" + filename + ".gz";

            memset(buffer, 0, sizeof(buffer));

            if (remove(filepath.c_str()) != 0) {
                std::cerr << "Failed to delete file: " << std::strerror(errno) << "\n";
                std::string errorMsg = "ERROR: Failed to delete file";
                send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
                continue;
            }

            strcpy(buffer, "success");
            int bytesSent = send(clientfd, "", sizeof(buffer), 0);
            if (bytesSent == -1) {
                std::cerr << "failed sending message: " << std::strerror(errno) << "\n";
                continue;
            }

        } else {
            std::cerr << "Unknown option selected by client\n";
        }

        std::cout << "\n";
    }
    close(clientfd);
}

int main() {
    struct stat st;
    memset(&st, 0, sizeof(st));

    if (stat("./store", &st) == -1) {
        mkdir("./store", 0700);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // server fd
    if (sockfd == -1) {
        std::cerr << "socket creation failed: " << std::strerror(errno) << "\n";
        return 1;
    }
    std::cout << "Socket created\n";

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int yes = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {  // to avoid "port already in use" error
        std::cerr << "setsockopt() failed: " << std::strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "failed binding to port: " << PORT << " " << std::strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        std::cerr << "failed listening: " << std::strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    std::cout << "Server listening on port: " << PORT << "\n";

    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    Threadpool pool(16);

    while (1) {
        int clientfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);  // client fd

        if (clientfd == -1) {
            std::cerr << "failed accept(): " << std::strerror(errno) << "\n";
            continue;
        }

        std::cout << "client connected: Address=" << inet_ntoa(clientAddr.sin_addr) << " Port=" << ntohs(clientAddr.sin_port) << "\n";

        pool.enqueueTask(clientHandler, clientfd);
    }

    close(sockfd);

    return 0;
}