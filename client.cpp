#include <arpa/inet.h>
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

#include "./include/progressbar.hpp"

#define PORT 7000
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // server fd
    if (sockfd == -1) {
        std::cerr << "socket creation failed: " << std::strerror(errno) << "\n";
        return 1;
    }
    std::cout << "Socket created\n";

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "failed connect(): " << std::strerror(errno) << "\n";
    }
    std::cout << "server connected\n";

    // char buffer[BUFFER_SIZE];

    // send name
    std::cout << "Enter your name: \n";
    std::string buffer;
    std::string name;
    std::cin >> buffer;
    if (buffer.length() == 0) {
        std::cerr << "enter valid name"
                  << "\n";
        close(sockfd);
        return 1;
    }

    // std::cout<<buffer<<"\n";

    int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending: " << std::strerror(errno) << "\n";
        close(sockfd);
        return 1;
    }

    name = buffer;

    // send option

    int option = 0;

    while (1) {
        std::cout << "Choose:\n 1. list files \t 2. upload file  \t 3. download file :\n";
        std::cin >> option;
        buffer.clear();

        if (option == 1) {  // ls
            // send ls
            buffer = "ls";
            bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

            if (bytesSent == -1) {
                std::cerr << "failed sending: " << std::strerror(errno) << "\n";
                close(sockfd);
                return 1;
            }
            buffer.clear();

            char recvBuffer[BUFFER_SIZE];
            int bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer) - 1, 0);
            if (bytesRecv <= 0) {
                std::cerr << "Failed receiving: " << std::strerror(errno) << "\n";
            }

            recvBuffer[bytesRecv] = '\0';
            std::cout << "Directory files: \n" << recvBuffer << "\n";

        } else if (option == 2) {  // upload

            // send upload
            buffer = "upload";
            bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

            if (bytesSent == -1) {
                std::cerr << "failed sending upload request: " << std::strerror(errno) << "\n";
                // close(sockfd);
                continue;
            }
            buffer.clear();

            std::string pathname;
            std::cout << "Enter file pathname: \n";
            std::cin >> pathname;

            int filefd = open(pathname.c_str(), O_RDONLY);
            if (filefd == -1) {
                std::cerr << "failed opening file: " << std::strerror(errno) << "\n";
                close(filefd);
                continue;
            }

            std::string filename = pathname.substr(pathname.find_last_of("/") + 1);

            off_t filesize = lseek(filefd, 0, SEEK_END);
            lseek(filefd, 0, SEEK_SET);

            std::string metadata = filename + ":" + std::to_string(filesize);

            bytesSent = send(sockfd, metadata.c_str(), metadata.length(), 0);

            if (bytesSent == -1) {
                std::cerr << "failed sending metadata: " << std::strerror(errno) << "\n";
                continue;
            }

            char ACK[3];

            int bytesRecv = recv(sockfd, ACK, sizeof(ACK) - 1, 0);

            ACK[bytesRecv] = '\0';

            if (bytesRecv == -1) {
                std::cerr << "failed recving metadata ACK: " << std::strerror(errno) << "\n";
                continue;
            }

            std::cout << "ACK: " << ACK << "\n";

            if (strcmp(ACK, "OK") != 0) {
                std::cerr << "recieved NACK: "
                          << "\n";
                continue;
            }

            int totalSteps = 100;
            ProgressBar progressBar(totalSteps);
            off_t totalBytesSent = 0;
            off_t bytesRemaining = filesize;
            const size_t chunkSize = 65536;

            while (bytesRemaining > 0) {
                size_t remaining = std::min(chunkSize, static_cast<size_t>(bytesRemaining));
                bytesSent = sendfile(sockfd, filefd, &totalBytesSent, remaining);
                if (bytesSent == -1) {
                    std::cerr << "failed sending file: " << std::strerror(errno) << "\n";
                    close(filefd);
                    break;
                }

                bytesRemaining -= bytesSent;

                int progress = static_cast<int>((static_cast<float>(totalBytesSent) / filesize) * totalSteps);
                progress = std::min(progress, totalSteps);
                progressBar.update(progress);
            }

            std::cout << "\n";

            if (bytesSent == -1) {
                std::cerr << "failed sending file: " << std::strerror(errno) << "\n";
                close(filefd);
                continue;
            }

            std::cout << "File sent\n";

            buffer.clear();
        } else if (option == 3) {  // download

            buffer = "download";
            bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

            if (bytesSent == -1) {
                std::cerr << "failed sending download request: " << std::strerror(errno) << "\n";
                // close(sockfd);
                continue;
            }
            buffer.clear();

            std::string filename;
            std::cout << "Enter file filename to download: \n";
            std::cin >> filename;

            std::string downloadPath;
            std::cout << "Enter pathname to download file: \n";
            std::cin >> downloadPath;
            std::string fullPath = downloadPath;

            if (downloadPath.back() != '/') {
                fullPath += "/";
            }
            fullPath += filename;

            bytesSent = send(sockfd, filename.c_str(), filename.length(), 0);  // send filename
            if (bytesSent == -1) {
                std::cerr << "Failed to send filename: " << std::strerror(errno) << "\n";
                continue;
            }


            char recvBuffer[BUFFER_SIZE];
            int bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);

            if (bytesRecv == -1) {
                std::cerr << "failed recieving compressed filesize: " << std::strerror(errno) << "\n";
                continue;
            }

            int filesize = atoi(recvBuffer);
            memset(recvBuffer, 0, sizeof(recvBuffer));

            int bytesSent = send(sockfd, "OK", 2, 0);
            if (bytesSent == -1) {
                std::cerr << "Failed sending ACK: " << std::strerror(errno) << "\n";
                break;
            }

            int downloadFd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (downloadFd == -1) {
                std::cerr << "Failed to create file: " << fullPath << " - " << std::strerror(errno) << "\n";
                continue;
            }

            int totalSteps = 100;
            ProgressBar progressBar(totalSteps);
            off_t totalBytesRecv = 0;
            off_t bytesRemaining = filesize;

            // zlib init
            z_stream zstream;
            zstream.zalloc = Z_NULL;
            zstream.zfree = Z_NULL;
            zstream.opaque = Z_NULL;
            zstream.avail_in = 0;
            zstream.next_in = Z_NULL;

            if (inflateInit2(&zstream, 16 + MAX_WBITS) != Z_OK) {
                std::cerr << "Failed to initialize zlib for decompression\n";
                close(downloadFd);
                continue;
            }

            while (bytesRemaining > 0) {
                bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);

                if (bytesRecv == -1) {
                    std::cerr << "failed recieving compressed file: " << std::strerror(errno) << "\n";
                    close(downloadFd);
                    inflateEnd(&zstream);
                    break;
                }

                totalBytesRecv += bytesRecv;
                bytesRemaining -= bytesRecv;

                zstream.avail_in = bytesRecv;
                zstream.next_in = reinterpret_cast<unsigned char *>(recvBuffer);

                unsigned char outBuffer[CHUNK_SIZE];
                do {
                    zstream.avail_out = CHUNK_SIZE;
                    zstream.next_out = outBuffer;

                    int ret = inflate(&zstream, Z_NO_FLUSH);
                    if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
                        std::cerr << "Decompression error: " << ret << "\n";
                        inflateEnd(&zstream);
                        close(downloadFd);
                        break;
                    }

                    int decompressedBytes = CHUNK_SIZE - zstream.avail_out;
                    if (write(downloadFd, outBuffer, decompressedBytes) == -1) { // Write decompressed data to file 
                        std::cerr << "Failed writing to file: " << std::strerror(errno) << "\n";
                        inflateEnd(&zstream);
                        close(downloadFd);
                        break;
                    }

                } while (zstream.avail_in > 0 ||  zstream.avail_out == 0 );

                int progress = static_cast<int>((static_cast<float>(totalBytesRecv) / filesize) * totalSteps);
                progress = std::min(progress, totalSteps);
                progressBar.update(progress);
            }
            std::cout << "\n";
            std::cout << "Recieved file: " << filename << "\n";

        } else {
            std::cerr << "invalid option"
                      << "\n";
            close(sockfd);
            return 1;
            // continue;
        }

        std::cout << "\n";
    }

    close(sockfd);

    return 0;
}