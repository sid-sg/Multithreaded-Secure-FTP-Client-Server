#include "../include/handlers.hpp"

#include <sys/socket.h>
#include <sys/types.h>

#include "../include/file_utils.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

namespace handlers {

// void clientHandler(int clientfd) {
//     std::vector<char> buffer(BUFFER_SIZE, 0);

//     int bytesRecv = recv(clientfd, buffer.data(), buffer.size() - 1, 0);  // recv clientname
//     if (bytesRecv <= 0) {
//         std::cerr << "Client disconnected or error occurred: " << std::strerror(errno) << "\n";
//         return;
//     }

//     buffer[bytesRecv] = '\0';
//     std::string clientname(buffer.data());
//     if (clientname.empty() || clientname.find('/') != std::string::npos) {
//         std::cerr << "Invalid client name received\n";
//         return;
//     }

//     std::cout << "Received from client: \n";
//     std::cout << "Name: " << clientname << "\n";

//     std::string folderdir = "../../store/" + clientname;

//     if (!utils::ensureDirectory(folderdir)) {
//         std::cerr << "Failed to create store directory\n";
//         return;
//     }

//     while (1) {
//         std::fill(buffer.begin(), buffer.end(), 0);

//         bytesRecv = recv(clientfd, buffer.data(), buffer.size() - 1, 0);  // recv option
//         if (bytesRecv <= 0) {
//             std::cerr << "Client disconnected or error occurred: " << std::strerror(errno) << "\n";
//             break;
//         }

//         buffer[bytesRecv] = '\0';
//         std::string option(buffer.data());
//         if (option.empty()) {
//             std::cerr << "Empty or invalid option received\n";
//             continue;
//         }

//         std::cout << "Option selected: " << option << "\n";

//         try {
//             if (option == "ls") {
//                 handlers::listFiles(clientfd, folderdir);
//             } else if (option == "upload") {
//                 handlers::uploadFile(clientfd, folderdir);
//             } else if (option == "download") {
//                 handlers::downloadFile(clientfd, folderdir);
//             } else if (option == "rename") {
//                 handlers::renameFile(clientfd, folderdir);
//             } else if (option == "delete") {
//                 handlers::deleteFile(clientfd, folderdir);
//             } else {
//                 std::cerr << "Unknown option selected by client\n";
//                 const std::string errorMsg = "ERROR: Unknown option\n";
//                 send(clientfd, errorMsg.c_str(), errorMsg.size(), 0);
//             }
//         } catch (const std::exception& e) {
//             std::cerr << "Exception during handling client option: " << e.what() << "\n";
//         } catch (...) {
//             std::cerr << "Unknown error occurred during handling client option\n";
//         }

//         std::cout << "\n";
//     }

//     close(clientfd);
// }


void clientHandler(SSL *ssl) {
    std::vector<char> buffer(BUFFER_SIZE, 0);

    int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size() - 1);  // recv clientname
    if (bytesRecv <= 0) {
        std::cerr << "Client disconnected or error occurred: " << std::strerror(errno) << "\n";
        return;
    }

    buffer[bytesRecv] = '\0';
    std::string clientname(buffer.data());
    if (clientname.empty() || clientname.find('/') != std::string::npos) {
        std::cerr << "Invalid client name received\n";
        return;
    }

    std::cout << "Received from client: \n";
    std::cout << "Name: " << clientname << "\n";

    std::string folderdir = "../../store/" + clientname;

    if (!utils::ensureDirectory(folderdir)) {
        std::cerr << "Failed to create store directory\n";
        return;
    }

    while (1) {
        std::fill(buffer.begin(), buffer.end(), 0);

        bytesRecv = SSL_read(ssl, buffer.data(), buffer.size() - 1);  // recv option
        if (bytesRecv <= 0) {
            std::cerr << "Client disconnected or error occurred: " << std::strerror(errno) << "\n";
            break;
        }

        buffer[bytesRecv] = '\0';
        std::string option(buffer.data());
        if (option.empty()) {
            std::cerr << "Empty or invalid option received\n";
            continue;
        }

        std::cout << "Option selected: " << option << "\n";

        try {
            if (option == "ls") {
                // handlers::listFiles(clientfd, folderdir);
            } else if (option == "upload") {
                // handlers::uploadFile(clientfd, folderdir);
            } else if (option == "download") {
                // handlers::downloadFile(clientfd, folderdir);
            } else if (option == "rename") {
                // handlers::renameFile(clientfd, folderdir);
            } else if (option == "delete") {
                // handlers::deleteFile(clientfd, folderdir);
            } else {
                std::cerr << "Unknown option selected by client\n";
                const std::string errorMsg = "ERROR: Unknown option\n";
                SSL_write(ssl, errorMsg.c_str(), errorMsg.size());
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception during handling client option: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "Unknown error occurred during handling client option\n";
        }

        std::cout << "\n";
    }

    // close(clientfd);
}

// void listFiles(int clientfd, const std::string& folderdir) {
//     std::string files = utils::ls(folderdir);

//     if (files.empty()) {
//         files = "Empty directory";
//     }

//     std::vector<char> buffer(files.begin(), files.end());

//     buffer.push_back('\0');

//     size_t totalSize = buffer.size();
//     size_t bytesSent = 0;
//     std::cout << "Sending directory files to client...\n";

//     // send files list in chunk
//     while (bytesSent < totalSize) {
//         size_t chunkSize = std::min(BUFFER_SIZE, static_cast<int>(totalSize - bytesSent));
//         ssize_t result = send(clientfd, buffer.data() + bytesSent, chunkSize, 0);

//         if (result == -1) {
//             std::cerr << "Failed to send directory files: " << std::strerror(errno) << "\n";
//             close(clientfd);
//             return;
//         }

//         bytesSent += result;
//     }

//     std::cout << "Successfully sent directory files to client.\n";
// }

// void uploadFile(int clientfd, const std::string& folderdir) {
//     std::vector<char> buffer(BUFFER_SIZE, 0);

//     // recv file metadata (filename:filesize)
//     int bytesRecv = recv(clientfd, buffer.data(), buffer.size(), 0);
//     if (bytesRecv <= 0) {
//         std::cerr << "Client disconnected or error receiving metadata: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::string metadata(buffer.data(), bytesRecv);

//     // Validate and parse metadata
//     size_t partition = metadata.find(":");
//     if (partition == std::string::npos) {
//         std::cerr << "Invalid metadata format received from client\n";
//         return;
//     }

//     std::string filename = metadata.substr(0, partition);
//     std::string filesizeStr = metadata.substr(partition + 1);

//     // validate filename
//     std::regex validFilenameRegex("^[a-zA-Z0-9._-]+$");
//     if (!std::regex_match(filename, validFilenameRegex)) {
//         std::cerr << "Invalid filename received: " << filename << "\n";
//         return;
//     }

//     // validate filesize
//     int filesize = 0;
//     try {
//         filesize = std::stoi(filesizeStr);
//         if (filesize <= 0) {
//             throw std::invalid_argument("Filesize must be positive");
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Invalid filesize received: " << filesizeStr << " (" << e.what() << ")\n";
//         return;
//     }

//     std::cout << "Filename: " << filename << "\n";
//     std::cout << "Filesize: " << filesize << " bytes\n";

//     // send ACKt
//     if (send(clientfd, "OK", 2, 0) == -1) {
//         std::cerr << "Failed sending ACK: " << std::strerror(errno) << "\n";
//         return;
//     }

//     // prepare write to compressed file
//     std::string compressedFilepath = folderdir + "/" + filename + ".gz";
//     gzFile compressedfile = gzopen(compressedFilepath.c_str(), "wb");
//     if (!compressedfile) {
//         std::cerr << "Failed to create compressed file: " << std::strerror(errno) << "\n";
//         return;
//     }

//     // recv file content in chunks
//     int remaining = filesize;
//     while (remaining > 0) {
//         buffer.assign(BUFFER_SIZE, 0);

//         int chunkSize = std::min(remaining, BUFFER_SIZE);
//         bytesRecv = recv(clientfd, buffer.data(), chunkSize, 0);
//         if (bytesRecv <= 0) {
//             std::cerr << "Client disconnected during file transfer: " << std::strerror(errno) << "\n";
//             gzclose(compressedfile);
//             return;
//         }

//         // write to compressed file
//         int bytesWritten = gzwrite(compressedfile, buffer.data(), bytesRecv);
//         if (bytesWritten != bytesRecv) {
//             std::cerr << "Error writing to compressed file\n";
//             gzclose(compressedfile);
//             return;
//         }

//         remaining -= bytesRecv;
//     }

//     gzclose(compressedfile);
//     std::cout << "File received and saved to " << compressedFilepath << "\n";
// }

// void downloadFile(int clientfd, const std::string& folderdir) {
//     std::vector<char> buffer(BUFFER_SIZE, 0);

//     // recv filename
//     int bytesRecv = recv(clientfd, buffer.data(), buffer.size(), 0);
//     if (bytesRecv <= 0) {
//         std::cerr << "Client disconnected or error receiving filename: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::string filename(buffer.data(), bytesRecv);
//     filename += ".gz";
//     std::string filepath = folderdir + "/" + filename;

//     int filefd = open(filepath.c_str(), O_RDONLY);
//     if (filefd == -1) {
//         std::cerr << "File not found: " << filename << "\n";
//         std::string errorMsg = "ERROR: File Not Found";
//         if (send(clientfd, errorMsg.c_str(), errorMsg.size(), 0) == -1) {
//             std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     off_t filesize = utils::getFilesize(filefd);
//     if (filesize == -1) {
//         std::cerr << "Failed to get file size: " << std::strerror(errno) << "\n";
//         close(filefd);
//         return;
//     }

//     // send filesize
//     std::string fileSizeMsg = std::to_string(filesize);
//     if (send(clientfd, fileSizeMsg.c_str(), fileSizeMsg.size(), 0) == -1) {
//         std::cerr << "Failed to send compressed file size: " << std::strerror(errno) << "\n";
//         close(filefd);
//         return;
//     }

//     // recv ACK
//     buffer.assign(BUFFER_SIZE, 0);
//     bytesRecv = recv(clientfd, buffer.data(), 2, 0);  // Expecting "OK"
//     if (bytesRecv <= 0 || std::string(buffer.data(), bytesRecv) != "OK") {
//         std::cerr << "Client failed to ACK file size: " << std::strerror(errno) << "\n";
//         close(filefd);
//         return;
//     }

//     // sendfile in chunks
//     off_t totalBytesSent = 0;
//     off_t bytesRemaining = filesize;
//     const size_t chunkSize = 65536;

//     while (bytesRemaining > 0) {
//         size_t remaining = std::min(chunkSize, static_cast<size_t>(bytesRemaining));
//         ssize_t bytesSent = sendfile(clientfd, filefd, &totalBytesSent, remaining);
//         if (bytesSent == -1) {
//             std::cerr << "Failed to send file: " << std::strerror(errno) << "\n";
//             close(filefd);
//             return;
//         }
//         bytesRemaining -= bytesSent;
//     }

//     std::cout << "Compressed file sent to client successfully.\n";

//     close(filefd);
// }

// void renameFile(int clientfd, const std::string& folderdir) {
//     std::vector<char> buffer(BUFFER_SIZE, 0);

//     // recv file metadata
//     int bytesRecv = recv(clientfd, buffer.data(), buffer.size(), 0);
//     if (bytesRecv <= 0) {
//         std::cerr << "Client disconnected or error receiving metadata: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::string metadata(buffer.data(), bytesRecv);

//     std::string::size_type partition = metadata.find(":");
//     if (partition == std::string::npos) {
//         std::cerr << "Invalid metadata received: " << metadata << "\n";
//         std::string errorMsg = "ERROR: Invalid metadata format";
//         if (send(clientfd, errorMsg.c_str(), errorMsg.size(), 0) == -1) {
//             std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     std::string filename = metadata.substr(0, partition);
//     std::string newname = metadata.substr(partition + 1);

//     std::string filepath = folderdir + "/" + filename + ".gz";
//     std::string newpath = folderdir + "/" + newname + ".gz";

//     std::cout << "Old path: " << filepath << "\n";
//     std::cout << "New path: " << newpath << "\n";

//     // rename file
//     if (rename(filepath.c_str(), newpath.c_str()) != 0) {
//         std::cerr << "Failed to rename file: " << std::strerror(errno) << "\n";
//         std::string errorMsg = "ERROR: Failed to rename file";
//         if (send(clientfd, errorMsg.c_str(), errorMsg.size(), 0) == -1) {
//             std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     // send response
//     std::string successMsg = "SUCCESS";
//     if (send(clientfd, successMsg.c_str(), successMsg.size(), 0) == -1) {
//         std::cerr << "Failed to send success message: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::cout << "File renamed successfully.\n";
// }

// void deleteFile(int clientfd, const std::string& folderdir) {
//     // recv filename
//     std::vector<char> buffer(BUFFER_SIZE, 0);
//     int bytesRecv = recv(clientfd, buffer.data(), buffer.size(), 0);
//     if (bytesRecv <= 0) {
//         std::cerr << "Client disconnected or error receiving filename: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::string filename(buffer.data(), bytesRecv);

//     std::string filepath = folderdir + "/" + filename + ".gz";

//     // delete file
//     if (remove(filepath.c_str()) != 0) {
//         std::cerr << "Failed to delete file: " << std::strerror(errno) << "\n";
//         std::string errorMsg = "ERROR: Failed to delete file";
//         if (send(clientfd, errorMsg.c_str(), errorMsg.size(), 0) == -1) {
//             std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     // send response
//     std::string successMsg = "SUCCESS";
//     if (send(clientfd, successMsg.c_str(), successMsg.size(), 0) == -1) {
//         std::cerr << "Failed to send success message: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::cout << "File successfully deleted: " << filepath << "\n";
// }
}  // namespace handlers