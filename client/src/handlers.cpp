#include "../include/handlers.hpp"

#include "../include/progressbar.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384

namespace handlers {

void serverHandler(SSL *ssl) {
    int option = 0;

    while (true) {
        std::cout << "Choose:\n"
                  << " 1. Register\n"
                  << " 2. Login\n"
                  << " 3. List files\n"
                  << " 4. Upload file\n"
                  << " 5. Download file\n"
                  << " 6. Rename file\n"
                  << " 7. Delete file\n"
                  << " 8. Exit\n"
                  << "Enter your choice: ";

        int option;
        if (!(std::cin >> option)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Invalid option. Please enter a number between 1 and 8.\n";
            continue;
        }

        std::cout<<option<<"\n";

        switch (option) {
            case 1:  // register
                registerUser(ssl);
                break;

            case 2:  // login
                loginUser(ssl);
                break;

            case 3:  // list files
                listFiles(ssl);
                break;

            case 4:  // upload file
                uploadFile(ssl);
                break;

            case 5:  // download file
                downloadFile(ssl);
                break;

            case 6:  // rename file
                renameFile(ssl);
                break;

            case 7:  // delete file
                deleteFile(ssl);
                break;

            case 8:  // exit
                std::cout << "Exiting...\n";
                return;

            default:
                std::cerr << "Invalid option. Please enter a number between 1 and 8.\n";
                break;
        }

        std::cout << "\n";
    }
}

void registerUser(SSL *ssl) {
    std::cout<<"Registering user\n";
    // send register req
    const std::string registerRequest = "register";
    if (SSL_write(ssl, registerRequest.c_str(), registerRequest.size()) <= 0) {
        std::cerr << "Failed to send register request: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }
    std::string username, password;

    std::cout << "Enter username: ";
    std::cin >> username;

    std::cout << "Enter password: ";
    std::cin >> password;

    if (!std::regex_match(username, usernameRegex)) {
        std::cerr << "Invalid username received: " << username << "\n";
        return;
    }
    if (!std::regex_match(password, passwordRegex)) {
        std::cerr << "Password does not meet the required criteria.\n";
        return;
    }

    // Send username
    if (SSL_write(ssl, username.c_str(), username.size()) <= 0) {
        std::cerr << "Failed to send username: " << std::strerror(errno) << "\n";
        return;
    }

    // Send password
    if (SSL_write(ssl, password.c_str(), password.size()) <= 0) {
        std::cerr << "Failed to send password: " << std::strerror(errno) << "\n";
        return;
    }

    // Wait to receive ACK
    std::string responseBuffer(1024, '\0');
    int bytesRecv = SSL_read(ssl, responseBuffer.data(), responseBuffer.size() - 1);
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Server closed the connection while waiting for ACK\n";
        } else {
            std::cerr << "Failed to receive ACK: " << std::strerror(errno) << "\n";
        }
        return;
    }

    responseBuffer.resize(bytesRecv);
    std::string response(responseBuffer.data(), bytesRecv);

    if (response == "OK") {
        std::cout << "User registered successfully.\n";
    } else if (response == "USER_EXISTS") {
        std::cerr << "Error: User already registered.\n";
    } else if (response == "INVALID_USERNAME_REGEX") {
        std::cerr << "Error: Invalid username format.\n";
    } else if (response == "INVALID_PASSWORD_REGEX") {
        std::cerr << "Error: Password does not meet the required criteria.\n";
    } else {
        std::cerr << "Error: Unknown response from server: " << response << "\n";
    }
}

void loginUser(SSL *ssl) {
    std::cout<<"Log in\n";
    // send login req
    const std::string uploadRequest = "login";
    if (SSL_write(ssl, uploadRequest.c_str(), uploadRequest.size()) <= 0) {
        std::cerr << "Failed to send login request: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    // Send username
    if (SSL_write(ssl, username.c_str(), username.size()) == -1) {
        std::cerr << "Failed to send username: " << std::strerror(errno) << "\n";
        return;
    }

    // Send password
    if (SSL_write(ssl, password.c_str(), password.size()) == -1) {
        std::cerr << "Failed to send password: " << std::strerror(errno) << "\n";
        return;
    }

    std::string responseBuffer(1024, '\0');
    int bytesRecv = SSL_read(ssl, responseBuffer.data(), responseBuffer.size() - 1);
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Server closed the connection while waiting for ACK\n";
        } else {
            std::cerr << "Failed to receive ACK: " << std::strerror(errno) << "\n";
        }
        return;
    }

    responseBuffer[bytesRecv] = '\0';
    std::string response(responseBuffer.data(), bytesRecv);

    if (response == "LOGIN_SUCCESS") {
        std::cout << "User logged in successfully.\n";
    } else if (response == "USER_NOT_FOUND") {
        std::cerr << "Error: User not found. Please register first.\n";
    } else if (response == "WRONG_PASSWORD") {
        std::cerr << "Error: Incorrect password. Please try again.\n";
    } else {
        std::cerr << "Error: Unknown response from server: " << response << "\n";
    }
}

bool checkLoggedIn(SSL *ssl) {
    std::string ackBuffer(1024, '\0');
    int bytesRecv = SSL_read(ssl, ackBuffer.data(), ackBuffer.size() - 1);
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Server closed the connection while waiting for ACK\n";
        } else {
            std::cerr << "Failed to receive ACK: " << std::strerror(errno) << "\n";
        }
        return false;
    }
    std::string response(ackBuffer.data(), bytesRecv);   
    if (response != "LOGIN_SUCCESS") {
        std::cerr << "Error: User not logged in. Please login first.\n";
        return false;
    }
    return true;
}

// function only performed if client is logged-in

void listFiles(SSL *ssl) {
    std::cout<<"List files\n";
    const std::string command = "ls";
    if (SSL_write(ssl, command.c_str(), command.size()) == -1) {
        std::cerr << "Failed to send command: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    if (!checkLoggedIn(ssl)) {
        return;
    }
    std::vector<char> recvBuffer(BUFFER_SIZE, 0);

    int bytesRecv = SSL_read(ssl, recvBuffer.data(), recvBuffer.size() - 1);
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Connection closed by server\n";
        } else {
            std::cerr << "Failed to receive data: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    recvBuffer[bytesRecv] = '\0';
    std::cout << "Directory files:\n" << recvBuffer.data() << "\n";
}

void uploadFile(SSL *ssl) {
    std::cout<<"Upload file\n";

    std::string pathname;
    std::cout << "Enter file pathname: \n";
    std::cin >> pathname;

    int filefd = open(pathname.c_str(), O_RDONLY);
    if (filefd == -1) {
        std::cerr << "Failed to open file: " << std::strerror(errno) << "\n";
        const std::string errorMsg = "error";
        if (SSL_write(ssl, errorMsg.c_str(), errorMsg.size()) <= 0) {
            std::cerr << "Failed to send error response to server: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    // send upload req
    const std::string uploadRequest = "upload";
    if (SSL_write(ssl, uploadRequest.c_str(), uploadRequest.size()) <= 0) {
        std::cerr << "Failed to send upload request: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        close(filefd);
        return;
    }

    if (!checkLoggedIn(ssl)) {
        return;
    }

    std::string filename = pathname.substr(pathname.find_last_of("/") + 1);

    off_t filesize = lseek(filefd, 0, SEEK_END);
    if (filesize == -1) {
        std::cerr << "Failed to get file size: " << std::strerror(errno) << "\n";
        close(filefd);
        return;
    }
    lseek(filefd, 0, SEEK_SET);

    // send metadata (filename:filesize)
    std::string metadata = filename + ":" + std::to_string(filesize);
    if (SSL_write(ssl, metadata.c_str(), metadata.size()) <= 0) {
        std::cerr << "Failed to send metadata: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        close(filefd);
        return;
    }

    // wait to recv ACK
    std::string ackBuffer(3, '\0');
    int bytesRecv = SSL_read(ssl, &ackBuffer[0], ackBuffer.size() - 1);
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Server closed the connection while waiting for ACK\n";
        } else {
            std::cerr << "Failed to receive ACK: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        close(filefd);
        return;
    }

    ackBuffer.resize(bytesRecv);
    if (ackBuffer != "OK") {
        std::cerr << "Received NACK from server\n";
        close(filefd);
        return;
    }

    bool isKernelTLS = (BIO_get_ktls_send(SSL_get_rbio(ssl)) > 0);
    if (isKernelTLS) {
        std::cerr << "Kernel TLS enabled, using SSL_sendfile for upload \n";
    } else {
        std::cerr << "Kernel TLS not enabled, falling back to read + SSL_write for file upload\n";
    }

    // inti progress bar
    const int totalSteps = 100;
    ProgressBar progressBar(totalSteps);

    // sendfile in chunks
    off_t totalBytesSent = 0;
    off_t bytesRemaining = filesize;
    const size_t chunkSize = 65536;

    // start timer
    auto startTime = std::chrono::high_resolution_clock::now();
    while (bytesRemaining > 0) {
        size_t toSend = std::min(static_cast<size_t>(bytesRemaining), chunkSize);
        ssize_t bytesSent = 0;

        if (isKernelTLS) {
            bytesSent = SSL_sendfile(ssl, filefd, totalBytesSent, toSend, 0);
        } else {
            std::vector<char> buffer(toSend);
            ssize_t readBytes = pread(filefd, buffer.data(), toSend, totalBytesSent);
            if (readBytes > 0) {
                bytesSent = SSL_write(ssl, buffer.data(), readBytes);
            }

            totalBytesSent += bytesSent;
        }
        if (bytesSent <= 0) {
            std::cerr << "Failed to send file chunk: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
            close(filefd);
            return;
        }

        bytesRemaining -= bytesSent;

        // update progress bar
        int progress = static_cast<int>((static_cast<double>(totalBytesSent) / filesize) * totalSteps);
        progressBar.update(std::min(progress, totalSteps));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    std::cout << "\nFile sent and compressed successfully in " << duration.count() << " seconds\n";
    close(filefd);
}

void downloadFile(SSL *ssl) {
    std::cout<<"Download file\n";

    std::string buffer = "download";
    // send download req
    int bytesSent = SSL_write(ssl, buffer.c_str(), buffer.length());

    if (bytesSent <= 0) {
        std::cerr << "Failed sending download request: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }
    buffer.clear();

    if (!checkLoggedIn(ssl)) {
        return;
    }
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

    bytesSent = SSL_write(ssl, filename.c_str(), filename.length());  // send filename
    if (bytesSent <= 0) {
        std::cerr << "Failed to send filename: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::vector<char> recvBuffer(BUFFER_SIZE);
    int bytesRecv = SSL_read(ssl, recvBuffer.data(), recvBuffer.size());

    if (bytesRecv <= 0) {
        std::cerr << "Failed receiving compressed file size: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::string serverResponse(recvBuffer.begin(), recvBuffer.begin() + bytesRecv);
    if (serverResponse.find("ERROR:") != std::string::npos) {
        std::cerr << "Server Error: " << serverResponse << "\n";
        return;
    }

    int filesize = std::stoi(serverResponse);
    std::fill(recvBuffer.begin(), recvBuffer.end(), 0);

    bytesSent = SSL_write(ssl, "OK", 2);
    if (bytesSent <= 0) {
        std::cerr << "Failed sending ACK: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
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

    // init zlib
    z_stream zstream{};
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

    // start timer
    auto startTime = std::chrono::high_resolution_clock::now();
    try {
        std::vector<unsigned char> decompressionBuffer(CHUNK_SIZE);
        while (bytesRemaining > 0) {
            bytesRecv = SSL_read(ssl, recvBuffer.data(), recvBuffer.size());  // recv compressed file data

            if (bytesRecv <= 0) {
                ERR_print_errors_fp(stderr);
                throw std::runtime_error("Failed receiving compressed file: " + std::string(std::strerror(errno)));
            }

            totalBytesRecv += bytesRecv;
            bytesRemaining -= bytesRecv;

            zstream.avail_in = bytesRecv;
            zstream.next_in = reinterpret_cast<unsigned char *>(recvBuffer.data());

            // decompress and write to file
            do {
                zstream.avail_out = decompressionBuffer.size();
                zstream.next_out = decompressionBuffer.data();

                int ret = inflate(&zstream, Z_NO_FLUSH);
                if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
                    throw std::runtime_error("Decompression error: " + std::to_string(ret));
                }

                size_t decompressedBytes = decompressionBuffer.size() - zstream.avail_out;
                if (write(downloadFd, decompressionBuffer.data(), decompressedBytes) == -1) {
                    throw std::runtime_error("Failed writing to file: " + std::string(std::strerror(errno)));
                }
            } while (zstream.avail_in > 0 || zstream.avail_out == 0);

            // update progress bar
            int progress = static_cast<int>((static_cast<float>(totalBytesRecv) / filesize) * totalSteps);
            progressBar.update(std::min(progress, totalSteps));
        }

        std::cout << "\nReceived file: " << filename << "\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
    } catch (...) {
        std::cerr << "An unknown error occurred.\n";
    }

    inflateEnd(&zstream);

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    std::cout << "File successfully recieved and uncompressed in: " << duration.count() << " seconds\n";
    close(downloadFd);
}

void renameFile(SSL *ssl) {
    std::cout<<"Rename file\n";

    // send rename req
    const std::string buffer = "rename";
    if (SSL_write(ssl, buffer.c_str(), buffer.length()) <= 0) {
        std::cerr << "Failed sending rename request: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    if (!checkLoggedIn(ssl)) {
        return;
    }
    std::string filename;
    std::cout << "Enter the filename to rename: ";
    std::cin >> filename;

    std::string newname;
    std::cout << "NOTE: Mention the file extension while giving the new name.\n";
    std::cout << "Enter new filename: ";
    std::cin >> newname;

    const std::string metadata = filename + ":" + newname;

    // send metadata
    if (SSL_write(ssl, metadata.c_str(), metadata.length()) <= 0) {
        std::cerr << "Failed to send metadata (filename and newname): " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::vector<char> recvBuffer(BUFFER_SIZE);

    // recv response
    int bytesRecv = SSL_read(ssl, recvBuffer.data(), recvBuffer.size() - 1);
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Connection closed by server.\n";
        } else {
            std::cerr << "Failed receiving server response: " << std::strerror(errno) << "\n";
        }
        ERR_print_errors_fp(stderr);
        return;
    }

    recvBuffer[bytesRecv] = '\0';

    std::string serverResponse(recvBuffer.begin(), recvBuffer.begin() + bytesRecv);
    if (serverResponse.find("ERROR:") != std::string::npos) {
        std::cerr << "Server Error: " << serverResponse << "\n";
        return;
    }

    std::cout << "File \"" << filename << "\" successfully renamed to \"" << newname << "\".\n";
}

void deleteFile(SSL *ssl) {
    std::cout<<"Delete file\n";

    // send delete req
    const std::string buffer = "delete";
    if (SSL_write(ssl, buffer.c_str(), buffer.length()) <= 0) {
        std::cerr << "Failed to send delete request: " << std::strerror(errno) << "\n";
        return;
    }

    if (!checkLoggedIn(ssl)) {
        return;
    }
    std::string filename;
    std::cout << "Enter the filename to delete: ";
    std::cin >> filename;

    // send filename
    if (SSL_write(ssl, filename.c_str(), filename.length()) <= 0) {
        std::cerr << "Failed to send filename: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::vector<char> recvBuffer(BUFFER_SIZE);

    // recv response
    int bytesRecv = SSL_read(ssl, recvBuffer.data(), recvBuffer.size() - 1);
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Connection closed by the server.\n";
        } else {
            std::cerr << "Failed to receive server response: " << std::strerror(errno) << "\n";
        }
        ERR_print_errors_fp(stderr);
        return;
    }

    recvBuffer[bytesRecv] = '\0';

    std::string serverResponse(recvBuffer.begin(), recvBuffer.begin() + bytesRecv);
    if (serverResponse.find("ERROR:") != std::string::npos) {
        std::cerr << "Server Error: " << serverResponse << "\n";
        return;
    }

    std::cout << "File \"" << filename << "\" successfully deleted.\n";
}

}  // namespace handlers
