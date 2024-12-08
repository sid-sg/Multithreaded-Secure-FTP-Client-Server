#include "handlers.hpp"

#include "utils.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

namespace handlers {

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
    memset(buffer, 0, sizeof(buffer));

    if (!utils::ensureDirectory(folderdir)) {
        std::cerr << "Failed to create store directory\n";
        return;
    }

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
            handlers::listFiles(clientfd, clientname);
        } else if (option == "upload") {
            handlers::uploadFile(clientfd, folderdir);
        } else if (option == "download") {
            handlers::downloadFile(clientfd, folderdir);
        } else if (option == "rename") {
            handlers::renameFile(clientfd, folderdir);
        } else if (option == "delete") {
            handlers::deleteFile(clientfd, folderdir);
        } else {
            std::cerr << "Unknown option selected by client\n";
        }

        std::cout << "\n";
    }
    close(clientfd);
}

void listFiles(int clientfd, const std::string& clientname) {
    std::string directory = std::string("./store/") + clientname;
    std::string files = utils::ls(directory);

    if (files.empty()) {
        files = "Empty directory";
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    if (files.size() < BUFFER_SIZE) {
        strcpy(buffer, files.c_str());
    } else {
        std::cerr << "Directory files is too large for buffer\n";
        close(clientfd);
        return;
    }

    // std::cout << "ls content: " << buffer;

    // Send the directory files to the client
    int bytesSent = send(clientfd, buffer, strlen(buffer), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed sending directory files: " << std::strerror(errno) << "\n";
        return;
    }
    std::cout << "Sent directory files to client\n";
}

void uploadFile(int clientfd, const std::string& folderdir) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv file metadata (filename:filesize)
    if (bytesRecv <= 0) {
        std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
        return;
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
        return;
    }

    std::string compressedFilepath = folderdir + "/" + filename + ".gz";

    gzFile compressedfile = gzopen(compressedFilepath.c_str(), "wb");
    if (!compressedfile) {
        std::cerr << "Failed creating compressed file: " << std::strerror(errno) << "\n";
        return;
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
}

void downloadFile(int clientfd, const std::string& folderdir) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv filename to download
    if (bytesRecv <= 0) {
        std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
        return;
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

    off_t filesize = utils::getFilesize(filefd);

    std::string data = std::to_string(filesize);
    int bytesSent = send(clientfd, data.c_str(), data.length(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed sending compressed filesize: " << std::strerror(errno) << "\n";
        return;
    }

    char ACK[3];

    bytesRecv = recv(clientfd, ACK, sizeof(ACK) - 1, 0);

    ACK[bytesRecv] = '\0';

    if (bytesRecv == -1) {
        std::cerr << "failed recving ACK: " << std::strerror(errno) << "\n";
        return;
    }

    std::cout << "ACK: " << ACK << "\n";

    if (strcmp(ACK, "OK") != 0) {
        std::cerr << "recieved NACK: "
                  << "\n";
        return;
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
}

void renameFile(int clientfd, const std::string& folderdir) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv file metadata
    if (bytesRecv <= 0) {
        std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
        return;
    }

    std::string metadata(buffer, bytesRecv);
    memset(buffer, 0, sizeof(buffer));

    std::string::size_type partition = metadata.find(":");  // Example usage
    if (partition == std::string::npos) {
        std::cerr << "Invalid metadata received: " << metadata << "\n";
        std::string errorMsg = "ERROR: Invalid metadata format";
        send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    std::string filename = metadata.substr(0, partition);
    std::string newname = metadata.substr(partition + 1);

    std::string filepath = folderdir + "/" + filename + ".gz";
    std::string newpath = folderdir + "/" + newname + ".gz";

    std::cout << "filepath: " << filepath << "\n";
    std::cout << "newpath: " << newpath << "\n";

    if (rename(filepath.c_str(), newpath.c_str()) != 0) {
        std::cerr << "Failed to rename file: " << std::strerror(errno) << "\n";
        std::string errorMsg = "ERROR: Failed to rename file";
        send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    strcpy(buffer, "success");
    int bytesSent = send(clientfd, "", sizeof(buffer), 0);
    if (bytesSent == -1) {
        std::cerr << "failed sending message: " << std::strerror(errno) << "\n";
        return;
    }
}

void deleteFile(int clientfd, const std::string& folderdir) {
    char buffer[BUFFER_SIZE];
    int bytesRecv = recv(clientfd, buffer, sizeof(buffer), 0);  // recv filename
    if (bytesRecv <= 0) {
        std::cerr << "client disconnected: " << std::strerror(errno) << "\n";
        return;
    }

    std::string filename(buffer, bytesRecv);
    std::string filepath = folderdir + "/" + filename + ".gz";

    memset(buffer, 0, sizeof(buffer));

    if (remove(filepath.c_str()) != 0) {
        std::cerr << "Failed to delete file: " << std::strerror(errno) << "\n";
        std::string errorMsg = "ERROR: Failed to delete file";
        send(clientfd, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    strcpy(buffer, "success");
    int bytesSent = send(clientfd, "", sizeof(buffer), 0);
    if (bytesSent == -1) {
        std::cerr << "failed sending message: " << std::strerror(errno) << "\n";
        return;
    }
}
}  // namespace handlers