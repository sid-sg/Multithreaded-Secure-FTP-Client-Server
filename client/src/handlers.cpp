#include "../include/handlers.hpp"

#include "../include/progressbar.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

namespace handlers {

// void serverHandler(int sockfd) {
//     std::string name;
//     std::cout << "Enter your name: ";
//     std::getline(std::cin, name);

//     if (name.empty()) {
//         std::cerr << "Invalid input: Name cannot be empty\n";
//         // close(sockfd);
//         return;
//     }

//     // send name
//     int bytesSent = send(sockfd, name.c_str(), name.length(), 0);

//     if (bytesSent == -1) {
//         std::cerr << "failed sending: " << std::strerror(errno) << "\n";
//         // close(sockfd);
//         return;
//     }

//     int option = 0;

//     while (true) {
//         std::cout << "Choose:\n"
//                   << " 1. List files\n"
//                   << " 2. Upload file\n"
//                   << " 3. Download file\n"
//                   << " 4. Rename file\n"
//                   << " 5. Delete file\n"
//                   << " 6. Exit\n"
//                   << "Enter your choice: ";

//         std::cin >> option;

//         switch (option) {
//             case 1:  // list files
//                 listFiles(sockfd);
//                 break;

//             case 2:  // upload file
//                 uploadFile(sockfd);
//                 break;

//             case 3:  // download file
//                 downloadFile(sockfd);
//                 break;

//             case 4:  // rename file
//                 renameFile(sockfd);
//                 break;

//             case 5:  // delete file
//                 deleteFile(sockfd);
//                 break;

//             case 6:  // exit
//                 std::cout << "Exiting...\n";
//                 // close(sockfd);
//                 return;

//             default:
//                 std::cerr << "Invalid option. Please enter a number between 1 and 6.\n";
//                 break;
//         }

//         std::cout << "\n";
//     }
// }

void serverHandler(SSL *ssl) {
    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    if (name.empty()) {
        std::cerr << "Invalid input: Name cannot be empty\n";
        // close(sockfd);
        return;
    }

    // send name
    int bytesSent = SSL_write(ssl, name.c_str(), name.length());

    if (bytesSent == -1) {
        std::cerr << "failed sending: " << std::strerror(errno) << "\n";
        // close(sockfd);
        return;
    }

    int option = 0;

    while (true) {
        std::cout << "Choose:\n"
                  << " 1. List files\n"
                  << " 2. Upload file\n"
                  << " 3. Download file\n"
                  << " 4. Rename file\n"
                  << " 5. Delete file\n"
                  << " 6. Exit\n"
                  << "Enter your choice: ";

        std::cin >> option;

        switch (option) {
            case 1:  // list files
                // listFiles(ssl);
                break;

            case 2:  // upload file
                // uploadFile(ssl);
                break;

            case 3:  // download file
                // downloadFile(ssl);
                break;

            case 4:  // rename file
                // renameFile(ssl);
                break;

            case 5:  // delete file
                // deleteFile(ssl);
                break;

            case 6:  // exit
                std::cout << "Exiting...\n";
                // close(sockfd);
                return;

            default:
                std::cerr << "Invalid option. Please enter a number between 1 and 6.\n";
                break;
        }

        std::cout << "\n";
    }
}

// void listFiles(int sockfd) {
//     const std::string command = "ls";
//     if (send(sockfd, command.c_str(), command.size(), 0) == -1) {
//         std::cerr << "Failed to send command: " << std::strerror(errno) << "\n";
//         // close(sockfd);
//         return;
//     }

//     std::vector<char> recvBuffer(BUFFER_SIZE, 0);

//     int bytesRecv = recv(sockfd, recvBuffer.data(), recvBuffer.size() - 1, 0);
//     if (bytesRecv <= 0) {
//         if (bytesRecv == 0) {
//             std::cerr << "Connection closed by server\n";
//         } else {
//             std::cerr << "Failed to receive data: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     recvBuffer[bytesRecv] = '\0';
//     std::cout << "Directory files:\n" << recvBuffer.data() << "\n";
// }

// void uploadFile(int sockfd) {
//     std::string pathname;
//     std::cout << "Enter file pathname: \n";
//     std::cin >> pathname;

//     int filefd = open(pathname.c_str(), O_RDONLY);
//     if (filefd == -1) {
//         std::cerr << "Failed to open file: " << std::strerror(errno) << "\n";
//         const std::string errorMsg = "error";
//         if (send(sockfd, errorMsg.c_str(), errorMsg.size(), 0) == -1) {
//             std::cerr << "Failed to send error response to server: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     // send upload req
//     const std::string uploadRequest = "upload";
//     if (send(sockfd, uploadRequest.c_str(), uploadRequest.size(), 0) == -1) {
//         std::cerr << "Failed to send upload request: " << std::strerror(errno) << "\n";
//         close(filefd);
//         return;
//     }

//     std::string filename = pathname.substr(pathname.find_last_of("/") + 1);

//     off_t filesize = lseek(filefd, 0, SEEK_END);
//     if (filesize == -1) {
//         std::cerr << "Failed to get file size: " << std::strerror(errno) << "\n";
//         close(filefd);
//         return;
//     }
//     lseek(filefd, 0, SEEK_SET);

//     // send metadata (filename:filesize)
//     std::string metadata = filename + ":" + std::to_string(filesize);
//     if (send(sockfd, metadata.c_str(), metadata.size(), 0) == -1) {
//         std::cerr << "Failed to send metadata: " << std::strerror(errno) << "\n";
//         close(filefd);
//         return;
//     }

//     // wait to recv ACK
//     std::string ackBuffer(3, '\0');
//     int bytesRecv = recv(sockfd, &ackBuffer[0], ackBuffer.size() - 1, 0);
//     if (bytesRecv <= 0) {
//         if (bytesRecv == 0) {
//             std::cerr << "Server closed the connection while waiting for ACK\n";
//         } else {
//             std::cerr << "Failed to receive ACK: " << std::strerror(errno) << "\n";
//         }
//         close(filefd);
//         return;
//     }

//     ackBuffer.resize(bytesRecv);
//     if (ackBuffer != "OK") {
//         std::cerr << "Received NACK from server\n";
//         close(filefd);
//         return;
//     }

//     // inti progress bar
//     const int totalSteps = 100;
//     ProgressBar progressBar(totalSteps);

//     // sendfile in chunks
//     off_t totalBytesSent = 0;
//     off_t bytesRemaining = filesize;
//     const size_t chunkSize = 65536;

//     // start timer
//     auto startTime = std::chrono::high_resolution_clock::now();
//     while (bytesRemaining > 0) {
//         size_t toSend = std::min(static_cast<size_t>(bytesRemaining), chunkSize);
//         ssize_t bytesSent = sendfile(sockfd, filefd, &totalBytesSent, toSend);
//         if (bytesSent == -1) {
//             std::cerr << "Failed to send file chunk: " << std::strerror(errno) << "\n";
//             close(filefd);
//             return;
//         }
//         bytesRemaining -= bytesSent;

//         // update progress bar
//         int progress = static_cast<int>((static_cast<double>(totalBytesSent) / filesize) * totalSteps);
//         progressBar.update(std::min(progress, totalSteps));
//     }

//     auto endTime = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> duration = endTime - startTime;

//     std::cout << "\nFile sent and compressed successfully in " << duration.count() << " seconds\n";
//     close(filefd);
// }

// void downloadFile(int sockfd) {
//     std::string buffer = "download";
//     // send download req
//     int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

//     if (bytesSent == -1) {
//         std::cerr << "Failed sending download request: " << std::strerror(errno) << "\n";
//         return;
//     }
//     buffer.clear();

//     std::string filename;
//     std::cout << "Enter file filename to download: \n";
//     std::cin >> filename;

//     std::string downloadPath;
//     std::cout << "Enter pathname to download file: \n";
//     std::cin >> downloadPath;
//     std::string fullPath = downloadPath;

//     if (downloadPath.back() != '/') {
//         fullPath += "/";
//     }
//     fullPath += filename;

//     bytesSent = send(sockfd, filename.c_str(), filename.length(), 0);  // send filename
//     if (bytesSent == -1) {
//         std::cerr << "Failed to send filename: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::vector<char> recvBuffer(BUFFER_SIZE);
//     int bytesRecv = recv(sockfd, recvBuffer.data(), recvBuffer.size(), 0);

//     if (bytesRecv <= 0) {
//         std::cerr << "Failed receiving compressed file size: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::string serverResponse(recvBuffer.begin(), recvBuffer.begin() + bytesRecv);
//     if (serverResponse.find("ERROR:") != std::string::npos) {
//         std::cerr << "Server Error: " << serverResponse << "\n";
//         return;
//     }

//     int filesize = std::stoi(serverResponse);
//     std::fill(recvBuffer.begin(), recvBuffer.end(), 0);

//     bytesSent = send(sockfd, "OK", 2, 0);
//     if (bytesSent == -1) {
//         std::cerr << "Failed sending ACK: " << std::strerror(errno) << "\n";
//         return;
//     }

//     int downloadFd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
//     if (downloadFd == -1) {
//         std::cerr << "Failed to create file: " << fullPath << " - " << std::strerror(errno) << "\n";
//         return;
//     }

//     int totalSteps = 100;
//     ProgressBar progressBar(totalSteps);
//     off_t totalBytesRecv = 0;
//     off_t bytesRemaining = filesize;

//     // init zlib
//     z_stream zstream{};
//     zstream.zalloc = Z_NULL;
//     zstream.zfree = Z_NULL;
//     zstream.opaque = Z_NULL;
//     zstream.avail_in = 0;
//     zstream.next_in = Z_NULL;

//     if (inflateInit2(&zstream, 16 + MAX_WBITS) != Z_OK) {
//         std::cerr << "Failed to initialize zlib for decompression\n";
//         close(downloadFd);
//         return;
//     }

//     // start timer
//     auto startTime = std::chrono::high_resolution_clock::now();
//     try {
//         std::vector<unsigned char> decompressionBuffer(CHUNK_SIZE);
//         while (bytesRemaining > 0) {
//             bytesRecv = recv(sockfd, recvBuffer.data(), recvBuffer.size(), 0);  // recv compressed file data

//             if (bytesRecv <= 0) {
//                 throw std::runtime_error("Failed receiving compressed file: " + std::string(std::strerror(errno)));
//             }

//             totalBytesRecv += bytesRecv;
//             bytesRemaining -= bytesRecv;

//             zstream.avail_in = bytesRecv;
//             zstream.next_in = reinterpret_cast<unsigned char *>(recvBuffer.data());

//             // decompress and write to file
//             do {
//                 zstream.avail_out = decompressionBuffer.size();
//                 zstream.next_out = decompressionBuffer.data();

//                 int ret = inflate(&zstream, Z_NO_FLUSH);
//                 if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
//                     throw std::runtime_error("Decompression error: " + std::to_string(ret));
//                 }

//                 size_t decompressedBytes = decompressionBuffer.size() - zstream.avail_out;
//                 if (write(downloadFd, decompressionBuffer.data(), decompressedBytes) == -1) {
//                     throw std::runtime_error("Failed writing to file: " + std::string(std::strerror(errno)));
//                 }
//             } while (zstream.avail_in > 0 || zstream.avail_out == 0);

//             // update progress bar
//             int progress = static_cast<int>((static_cast<float>(totalBytesRecv) / filesize) * totalSteps);
//             progressBar.update(std::min(progress, totalSteps));
//         }

//         std::cout << "\nReceived file: " << filename << "\n";
//     } catch (const std::exception &e) {
//         std::cerr << e.what() << "\n";
//     } catch (...) {
//         std::cerr << "An unknown error occurred.\n";
//     }

//     inflateEnd(&zstream);

//     auto endTime = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> duration = endTime - startTime;

//     std::cout << "File successfully recieved and uncompressed in: " << duration.count() << " seconds\n";
//     close(downloadFd);
// }

// void renameFile(int sockfd) {
//     // send rename req
//     const std::string buffer = "rename";
//     if (send(sockfd, buffer.c_str(), buffer.length(), 0) == -1) {
//         std::cerr << "Failed sending rename request: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::string filename;
//     std::cout << "Enter the filename to rename: ";
//     std::cin >> filename;

//     std::string newname;
//     std::cout << "NOTE: Mention the file extension while giving the new name.\n";
//     std::cout << "Enter new filename: ";
//     std::cin >> newname;

//     const std::string metadata = filename + ":" + newname;

//     // send metadata
//     if (send(sockfd, metadata.c_str(), metadata.length(), 0) == -1) {
//         std::cerr << "Failed to send metadata (filename and newname): " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::vector<char> recvBuffer(BUFFER_SIZE);

//     // recv response
//     int bytesRecv = recv(sockfd, recvBuffer.data(), recvBuffer.size() - 1, 0);
//     if (bytesRecv <= 0) {
//         if (bytesRecv == 0) {
//             std::cerr << "Connection closed by server.\n";
//         } else {
//             std::cerr << "Failed receiving server response: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     recvBuffer[bytesRecv] = '\0';

//     std::string serverResponse(recvBuffer.begin(), recvBuffer.begin() + bytesRecv);
//     if (serverResponse.find("ERROR:") != std::string::npos) {
//         std::cerr << "Server Error: " << serverResponse << "\n";
//         return;
//     }

//     std::cout << "File \"" << filename << "\" successfully renamed to \"" << newname << "\".\n";
// }

// void deleteFile(int sockfd) {
//     // send delete req
//     const std::string buffer = "delete";
//     if (send(sockfd, buffer.c_str(), buffer.length(), 0) == -1) {
//         std::cerr << "Failed to send delete request: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::string filename;
//     std::cout << "Enter the filename to delete: ";
//     std::cin >> filename;

//     // send filename
//     if (send(sockfd, filename.c_str(), filename.length(), 0) == -1) {
//         std::cerr << "Failed to send filename: " << std::strerror(errno) << "\n";
//         return;
//     }

//     std::vector<char> recvBuffer(BUFFER_SIZE);

//     // recv response
//     int bytesRecv = recv(sockfd, recvBuffer.data(), recvBuffer.size() - 1, 0);  
//     if (bytesRecv <= 0) {
//         if (bytesRecv == 0) {
//             std::cerr << "Connection closed by the server.\n";
//         } else {
//             std::cerr << "Failed to receive server response: " << std::strerror(errno) << "\n";
//         }
//         return;
//     }

//     recvBuffer[bytesRecv] = '\0';

//     std::string serverResponse(recvBuffer.begin(), recvBuffer.begin() + bytesRecv);
//     if (serverResponse.find("ERROR:") != std::string::npos) {
//         std::cerr << "Server Error: " << serverResponse << "\n";
//         return;
//     }

//     std::cout << "File \"" << filename << "\" successfully deleted.\n";
// }
}  // namespace handlers
