#include "../include/handlers.hpp"

#include "../include/progressbar.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

namespace handlers {

void serverHandler(int sockfd) {
    // send name
    std::cout << "Enter your name: \n";
    std::string buffer;
    std::string name;
    std::cin >> buffer;
    if (buffer.length() == 0) {
        std::cerr << "enter valid name"
                  << "\n";
        close(sockfd);
        return;
    }

    // std::cout<<buffer<<"\n";

    int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending: " << std::strerror(errno) << "\n";
        close(sockfd);
        return;
    }

    name = buffer;

    // send option

    int option = 0;

    while (1) {
        std::cout << "Choose:\n 1. list files \t 2. upload file \t 3. download file \t 4. rename file \t 5. delete file \t 6. exit :\n";
        std::cin >> option;
        buffer.clear();

        if (option == 1) {  // ls
            listFiles(sockfd);
        } else if (option == 2) {  // upload
            uploadFile(sockfd);
        } else if (option == 3) {  // download
            downloadFile(sockfd);
        } else if (option == 4) {  // rename
            renameFile(sockfd);

        } else if (option == 5) {  // delete
            deleteFile(sockfd);
        } else if (option == 6) {
            std::cerr << "exiting..."
                      << "\n";
            close(sockfd);
            return;
        } else {
            std::cerr << "invalid option"
                      << "\n";
            continue;
        }

        std::cout << "\n";
    }
}

void listFiles(int sockfd) {
    std::string buffer = "ls";
    int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending: " << std::strerror(errno) << "\n";
        close(sockfd);
        return;
    }
    buffer.clear();

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));
    int bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer) - 1, 0);
    if (bytesRecv <= 0) {
        std::cerr << "Failed receiving: " << std::strerror(errno) << "\n";
        return;
    }

    recvBuffer[bytesRecv] = '\0';
    std::cout << "Directory files: \n" << recvBuffer << "\n";
}

void uploadFile(int sockfd) {
    std::string buffer;
    std::string pathname;
    std::cout << "Enter file pathname: \n";
    std::cin >> pathname;
    int bytesSent = 0;
    int filefd = open(pathname.c_str(), O_RDONLY);
    if (filefd == -1) {
        std::cerr << "failed opening file: " << std::strerror(errno) << "\n";
        buffer = "error";
        bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);
        buffer.clear();
        return;
    }

    // send upload request
    buffer = "upload";
    bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending upload request: " << std::strerror(errno) << "\n";
        // close(sockfd);
        return;
    }
    buffer.clear();

    std::string filename = pathname.substr(pathname.find_last_of("/") + 1);

    off_t filesize = lseek(filefd, 0, SEEK_END);
    lseek(filefd, 0, SEEK_SET);

    std::string metadata = filename + ":" + std::to_string(filesize);

    bytesSent = send(sockfd, metadata.c_str(), metadata.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending metadata: " << std::strerror(errno) << "\n";
        return;
    }

    char ACK[3];

    memset(ACK, 0, sizeof(ACK));

    int bytesRecv = recv(sockfd, ACK, sizeof(ACK) - 1, 0);

    ACK[bytesRecv] = '\0';

    if (bytesRecv == -1) {
        std::cerr << "failed recving metadata ACK: " << std::strerror(errno) << "\n";
        return;
    }

    std::cout << "ACK: " << ACK << "\n";

    if (strcmp(ACK, "OK") != 0) {
        std::cerr << "recieved NACK: "
                  << "\n";
        return;
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
        return;
    }

    std::cout << "File sent\n";

    buffer.clear();
}

void downloadFile(int sockfd) {
    std::string buffer = "download";
    int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending download request: " << std::strerror(errno) << "\n";
        // close(sockfd);
        return;
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
        return;
    }

    char recvBuffer[BUFFER_SIZE];
    int bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);

    if (bytesRecv <= 0) {
        std::cerr << "failed recieving compressed filesize: " << std::strerror(errno) << "\n";
        return;
    }

    std::string serverResponse(recvBuffer);
    if (serverResponse.find("ERROR:") != std::string::npos) {
        std::cerr << "Server Error: " << serverResponse << "\n";
        return;
    }

    int filesize = atoi(recvBuffer);
    memset(recvBuffer, 0, sizeof(recvBuffer));

    bytesSent = send(sockfd, "OK", 2, 0);
    if (bytesSent == -1) {
        std::cerr << "Failed sending ACK: " << std::strerror(errno) << "\n";
        return;
    }

    int downloadFd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (downloadFd == -1) {
        std::cerr << "Failed to create file: " << fullPath << " - " << std::strerror(errno) << "\n";
        return;
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
        return;
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
            if (write(downloadFd, outBuffer, decompressedBytes) == -1) {  // Write decompressed data to file
                std::cerr << "Failed writing to file: " << std::strerror(errno) << "\n";
                inflateEnd(&zstream);
                close(downloadFd);
                break;
            }

        } while (zstream.avail_in > 0 || zstream.avail_out == 0);

        int progress = static_cast<int>((static_cast<float>(totalBytesRecv) / filesize) * totalSteps);
        progress = std::min(progress, totalSteps);
        progressBar.update(progress);
    }
    std::cout << "\n";
    std::cout << "Recieved file: " << filename << "\n";
}

void renameFile(int sockfd) {
    std::string buffer = "rename";
    int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending rename request: " << std::strerror(errno) << "\n";
        return;
    }
    buffer.clear();

    std::string filename;
    std::cout << "Enter file filename to rename: \n";
    std::cin >> filename;

    std::cout << "NOTE: Mention the file extension while giving new name\n";
    std::cout << "Enter new filename:\n";
    std::string newname;
    std::cin >> newname;

    std::string metadata = filename + ":";
    metadata += newname;

    std::cout << "metadata: " << metadata << "\n";

    bytesSent = send(sockfd, metadata.c_str(), metadata.length(), 0);  // send metadata( fielname:newfilename)
    if (bytesSent == -1) {
        std::cerr << "Failed to send filename: " << std::strerror(errno) << "\n";
        return;
    }

    char recvBuffer[BUFFER_SIZE];
    int bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer) - 1, 0);
    recvBuffer[bytesRecv] = '\0';

    if (bytesRecv <= 0) {
        std::cerr << "failed recieving server response: " << std::strerror(errno) << "\n";
        return;
    }

    std::string serverResponse(recvBuffer);
    if (serverResponse.find("ERROR:") != std::string::npos) {
        std::cerr << "Server Error: " << serverResponse << "\n";
        return;
    }

    std::cout << "File: " << filename << ", renamed to :" << newname << "\n";
}

void deleteFile(int sockfd) {
    std::string buffer = "delete";
    int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if (bytesSent == -1) {
        std::cerr << "failed sending delete request: " << std::strerror(errno) << "\n";
        return;
    }
    buffer.clear();

    std::string filename;
    std::cout << "Enter file filename to delete: \n";
    std::cin >> filename;

    bytesSent = send(sockfd, filename.c_str(), filename.length(), 0);  // send filename
    if (bytesSent == -1) {
        std::cerr << "Failed to send filename: " << std::strerror(errno) << "\n";
        return;
    }

    char recvBuffer[BUFFER_SIZE];
    int bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer) - 1, 0);
    recvBuffer[bytesRecv] = '\0';

    if (bytesRecv <= 0) {
        std::cerr << "failed recieving server response: " << std::strerror(errno) << "\n";
        return;
    }

    std::string serverResponse(recvBuffer);
    if (serverResponse.find("ERROR:") != std::string::npos) {
        std::cerr << "Server Error: " << serverResponse << "\n";
        return;
    }

    std::cout << "File: " << filename << " deleted"
              << "\n";
}
}  // namespace handlers
