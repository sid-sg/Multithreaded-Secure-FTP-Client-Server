#include "../include/handlers.hpp"

#include <sys/socket.h>
#include <sys/types.h>

#include "../include/db.hpp"
#include "../include/file_utils.hpp"
#include "../include/crypto.hpp"

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 16384
std::string folderdir = "\0";

void clientHandler::handler() {
    std::vector<char> buffer(BUFFER_SIZE, 0);

    while (1) {
        std::fill(buffer.begin(), buffer.end(), 0);

        int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size() - 1);  // recv option
        if (bytesRecv <= 0) {
            std::cerr << "Client disconnected or error occurred: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
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
            if (option == "register") {
                registerUser();
            } else if (option == "login") {
                loginUser();
            } else if (option == "ls") {
                listFiles();
            } else if (option == "upload") {
                uploadFile();
            } else if (option == "download") {
                downloadFile();
            } else if (option == "rename") {
                renameFile();
            } else if (option == "delete") {
                deleteFile();
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
}

void clientHandler::registerUser() {
    std::string username, password;

    // recv username
    std::vector<char> buffer(BUFFER_SIZE, 0);
    int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size() - 1);
    if (bytesRecv <= 0) {
        std::cerr << "Failed to receive username: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    buffer[bytesRecv] = '\0';
    username = buffer.data();

    // recv password
    bytesRecv = SSL_read(ssl, buffer.data(), buffer.size() - 1);
    if (bytesRecv <= 0) {
        std::cerr << "Failed to receive password: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    buffer[bytesRecv] = '\0';
    password = buffer.data();

    std::cout << "Received from client: \n";
    std::cout << "Username: " << username << "\n";
    std::cout << "Password: " << password << "\n";

    if (!std::regex_match(username, usernameRegex)) {
        std::cerr << "Invalid username received: " << username << "\n";
        const std::string errorMsg = "INVALID_USERNAME_REGEX";
        SSL_write(ssl, errorMsg.c_str(), errorMsg.size());
        return;
    }

    if (!std::regex_match(password, passwordRegex)) {
        std::cerr << "Invalid password received: " << password << "\n";
        const std::string errorMsg = "INVALID_PASSWORD_REGEX";
        SSL_write(ssl, errorMsg.c_str(), errorMsg.size());
        return;
    }

    Crypto crypto;

    std::string hashedPassword = crypto.hashPassword(password);
    std::cout << "Hashed password: " << hashedPassword << "\n";

    Database db("../data/user.db");

    if (db.userExists(username)) {
        const std::string errorMsg = "USER_EXISTS";
        SSL_write(ssl, errorMsg.c_str(), errorMsg.size());
        return;
    }

    if (!db.registerUser(username, hashedPassword)) {
        std::cerr << "Failed to register user\n";
        return;
    }

    folderdir = "../../store/" + username;

    if (!utils::ensureDirectory(folderdir)) {
        std::cerr << "Failed to create store directory\n";
        return;
    }

    // send ACK
    const std::string ack = "OK";
    if (SSL_write(ssl, ack.c_str(), ack.size()) <= 0) {
        std::cerr << "Failed to send ACK: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::cout << "User registered successfully: " << username << "\n";
}

void clientHandler::loginUser() {
    std::string username, password;

    // recv username
    std::vector<char> buffer(BUFFER_SIZE, 0);
    int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size() - 1);
    if (bytesRecv <= 0) {
        std::cerr << "Failed to receive username: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    buffer[bytesRecv] = '\0';
    username = buffer.data();

    // recv password
    bytesRecv = SSL_read(ssl, buffer.data(), buffer.size() - 1);
    if (bytesRecv <= 0) {
        std::cerr << "Failed to receive password: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }
    buffer[bytesRecv] = '\0';
    password = buffer.data();

    Database db("../data/user.db");
    if (!db.userExists(username)) {
        const std::string errorMsg = "USER_NOT_FOUND";
        SSL_write(ssl, errorMsg.c_str(), errorMsg.size());
        return;
    }
    std::string storedHash = db.getHashedPassword(username);
    std::cout<<"storegHash: "<<storedHash<<"\n";

    if (storedHash.empty()) {
        const std::string errorMsg = "USER_NOT_FOUND";
        SSL_write(ssl, errorMsg.c_str(), errorMsg.size());
        return;
    }

    Crypto crypto;

    if ( !crypto.verifyPasswords(storedHash, password) ) {
        const std::string errorMsg = "WRONG_PASSWORD";
        std::cout<<errorMsg<<"\n";
        SSL_write(ssl, errorMsg.c_str(), errorMsg.size());
        return;
    }

    folderdir = "../../store/" + username;

    if (!utils::ensureDirectory(folderdir)) {
        std::cerr << "Failed to create store directory\n";
        return;
    }

    isLoggedin = true;

    // Send ack
    const std::string ack = "LOGIN_SUCCESS";
    SSL_write(ssl, ack.c_str(), ack.size());

    std::cout << "User logged in successfully: " << username << "\n";
}

void clientHandler::sendLoggedInStatus() {
    if (isLoggedin) {
        const std::string ack = "LOGIN_SUCCESS";
        if (SSL_write(ssl, ack.c_str(), ack.size()) <= 0) {
            std::cerr << "Failed to send ACK: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
            return;
        }
    } else {
        const std::string ack = "LOGIN_FAIL";
        if (SSL_write(ssl, ack.c_str(), ack.size()) <= 0) {
            std::cerr << "Failed to send ACK: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
            return;
        }
    }
}

// function only performed if client is logged-in

void clientHandler::listFiles() {
    sendLoggedInStatus();
    if (!isLoggedin) {
        std::cout << "User not logged in\n";
        return;
    }
    std::cout << "user logged in\n";

    std::string files = utils::ls(folderdir);

    if (files.empty()) {
        files = "Empty directory";
    }

    std::vector<char> buffer(files.begin(), files.end());

    buffer.push_back('\0');

    size_t totalSize = buffer.size();
    size_t bytesSent = 0;
    std::cout << "Sending directory files to client...\n";

    // send files list in chunk
    while (bytesSent < totalSize) {
        size_t chunkSize = std::min(BUFFER_SIZE, static_cast<int>(totalSize - bytesSent));
        ssize_t result = SSL_write(ssl, buffer.data() + bytesSent, chunkSize);

        if (result == -1) {
            std::cerr << "Failed to send directory files: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            return;
        }

        bytesSent += result;
    }

    std::cout << "Successfully sent directory files to client.\n";
}

void clientHandler::uploadFile() {
    sendLoggedInStatus();
    if (!isLoggedin) {
        std::cout << "User not logged in\n";
        return;
    }
    std::cout << "user logged in\n";

    std::vector<char> buffer(BUFFER_SIZE, 0);

    // recv file metadata (filename:filesize)
    int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size());
    if (bytesRecv <= 0) {
        std::cerr << "Client disconnected or error receiving metadata: " << std::strerror(errno) << "\n";
        return;
    }

    std::string metadata(buffer.data(), bytesRecv);

    // Validate and parse metadata
    size_t partition = metadata.find(":");
    if (partition == std::string::npos) {
        std::cerr << "Invalid metadata format received from client\n";
        return;
    }

    std::string filename = metadata.substr(0, partition);
    std::string filesizeStr = metadata.substr(partition + 1);

    // validate filename
    std::regex validFilenameRegex("^[a-zA-Z0-9._-]+$");
    if (!std::regex_match(filename, validFilenameRegex)) {
        std::cerr << "Invalid filename received: " << filename << "\n";
        return;
    }

    // validate filesize
    int filesize = 0;
    try {
        filesize = std::stoi(filesizeStr);
        if (filesize <= 0) {
            throw std::invalid_argument("Filesize must be positive");
        }
    } catch (const std::exception& e) {
        std::cerr << "Invalid filesize received: " << filesizeStr << " (" << e.what() << ")\n";
        return;
    }

    std::cout << "Filename: " << filename << "\n";
    std::cout << "Filesize: " << filesize << " bytes\n";

    // send ACK
    if (SSL_write(ssl, "OK", 2) <= 0) {
        std::cerr << "Failed sending ACK: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    // prepare write to compressed file
    std::string compressedFilepath = folderdir + "/" + filename + ".gz";
    gzFile compressedfile = gzopen(compressedFilepath.c_str(), "wb");
    if (!compressedfile) {
        std::cerr << "Failed to create compressed file: " << std::strerror(errno) << "\n";
        return;
    }

    // recv file content in chunks
    int remaining = filesize;
    while (remaining > 0) {
        buffer.assign(BUFFER_SIZE, 0);

        int chunkSize = std::min(remaining, BUFFER_SIZE);
        bytesRecv = SSL_read(ssl, buffer.data(), chunkSize);
        if (bytesRecv <= 0) {
            std::cerr << "Client disconnected during file transfer: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
            gzclose(compressedfile);
            return;
        }

        // write to compressed file
        int bytesWritten = gzwrite(compressedfile, buffer.data(), bytesRecv);
        if (bytesWritten != bytesRecv) {
            std::cerr << "Error writing to compressed file\n";
            gzclose(compressedfile);
            return;
        }

        remaining -= bytesRecv;
    }

    gzclose(compressedfile);
    std::cout << "File received and saved to " << compressedFilepath << "\n";
}

void clientHandler::downloadFile() {
    sendLoggedInStatus();
    if (!isLoggedin) {
        std::cout << "User not logged in\n";
        return;
    }
    std::cout << "user logged in\n";
    std::vector<char> buffer(BUFFER_SIZE, 0);

    // Receive filename
    int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size());
    if (bytesRecv <= 0) {
        std::cerr << "Client disconnected or error receiving filename: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::string filename(buffer.data(), bytesRecv);
    filename += ".gz";
    std::string filepath = folderdir + "/" + filename;

    int filefd = open(filepath.c_str(), O_RDONLY);
    if (filefd == -1) {
        std::cerr << "File not found: " << filename << "\n";
        std::string errorMsg = "ERROR: File Not Found";
        if (SSL_write(ssl, errorMsg.c_str(), errorMsg.size()) <= 0) {
            std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    off_t filesize = utils::getFilesize(filefd);
    if (filesize == -1) {
        std::cerr << "Failed to get file size: " << std::strerror(errno) << "\n";
        close(filefd);
        return;
    }

    // Send filesize
    std::string fileSizeMsg = std::to_string(filesize);
    if (SSL_write(ssl, fileSizeMsg.c_str(), fileSizeMsg.size()) <= 0) {
        std::cerr << "Failed to send compressed file size: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        close(filefd);
        return;
    }

    // Receive ACK
    buffer.assign(BUFFER_SIZE, 0);
    bytesRecv = SSL_read(ssl, buffer.data(), 2);  // Expecting "OK"
    if (bytesRecv <= 0 || std::string(buffer.data(), bytesRecv) != "OK") {
        std::cerr << "Client failed to ACK file size: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        close(filefd);
        return;
    }

    // Check if Kernel TLS is enabled
    bool isKernelTLS = (BIO_get_ktls_send(SSL_get_rbio(ssl)) > 0);
    if (isKernelTLS) {
        std::cerr << "Kernel TLS enabled, using SSL_sendfile for download\n";
    } else {
        std::cerr << "Kernel TLS not enabled, falling back to read + SSL_write\n";
    }

    // Send file in chunks
    off_t totalBytesSent = 0;
    off_t bytesRemaining = filesize;
    const size_t chunkSize = 65536;

    while (bytesRemaining > 0) {
        size_t toSend = std::min(static_cast<size_t>(bytesRemaining), chunkSize);
        ssize_t bytesSent = 0;

        if (isKernelTLS) {  // Use SSL_sendfile for kTLS-enabled systems
            bytesSent = SSL_sendfile(ssl, filefd, totalBytesSent, toSend, 0);
        } else {  // Fallback: Use regular SSL_write for non-kTLS systems
            std::vector<char> buffer(toSend);
            ssize_t readBytes = pread(filefd, buffer.data(), toSend, totalBytesSent);
            if (readBytes > 0) {
                bytesSent = SSL_write(ssl, buffer.data(), readBytes);
            }
            totalBytesSent += bytesSent;
        }

        if (bytesSent <= 0) {
            std::cerr << "Failed to send file: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
            close(filefd);
            return;
        }

        bytesRemaining -= bytesSent;
    }

    std::cout << "Compressed file sent to client successfully.\n";

    close(filefd);
}

void clientHandler::renameFile() {
    sendLoggedInStatus();
    if (!isLoggedin) {
        std::cout << "User not logged in\n";
        return;
    }
    std::cout << "user logged in\n";
    std::vector<char> buffer(BUFFER_SIZE, 0);

    // Receive file metadata
    int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size());
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Client disconnected while receiving metadata.\n";
        } else {
            std::cerr << "Error receiving metadata: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    std::string metadata(buffer.data(), bytesRecv);

    std::string::size_type partition = metadata.find(":");
    if (partition == std::string::npos) {
        std::cerr << "Invalid metadata received: " << metadata << "\n";
        std::string errorMsg = "ERROR: Invalid metadata format";
        if (SSL_write(ssl, errorMsg.c_str(), errorMsg.size()) <= 0) {
            std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    std::string filename = metadata.substr(0, partition);
    std::string newname = metadata.substr(partition + 1);

    std::string filepath = folderdir + "/" + filename + ".gz";
    std::string newpath = folderdir + "/" + newname + ".gz";

    std::cout << "Old path: " << filepath << "\n";
    std::cout << "New path: " << newpath << "\n";

    // Rename file
    if (rename(filepath.c_str(), newpath.c_str()) != 0) {
        std::cerr << "Failed to rename file: " << std::strerror(errno) << "\n";
        std::string errorMsg = "ERROR: Failed to rename file";
        if (SSL_write(ssl, errorMsg.c_str(), errorMsg.size()) <= 0) {
            std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    // Send success response
    std::string successMsg = "SUCCESS";
    if (SSL_write(ssl, successMsg.c_str(), successMsg.size()) <= 0) {
        std::cerr << "Failed to send success message: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::cout << "File renamed successfully.\n";
}

void clientHandler::deleteFile() {
    sendLoggedInStatus();
    if (!isLoggedin) {
        std::cout << "User not logged in\n";
        return;
    }
    std::cout << "user logged in\n";
    // Receive filename
    std::vector<char> buffer(BUFFER_SIZE, 0);
    int bytesRecv = SSL_read(ssl, buffer.data(), buffer.size());
    if (bytesRecv <= 0) {
        if (bytesRecv == 0) {
            std::cerr << "Client disconnected while receiving filename.\n";
        } else {
            std::cerr << "Error receiving filename: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    std::string filename(buffer.data(), bytesRecv);

    std::string filepath = folderdir + "/" + filename + ".gz";

    // Delete file
    if (remove(filepath.c_str()) != 0) {
        std::cerr << "Failed to delete file: " << std::strerror(errno) << "\n";
        std::string errorMsg = "ERROR: Failed to delete file";
        if (SSL_write(ssl, errorMsg.c_str(), errorMsg.size()) <= 0) {
            std::cerr << "Failed to send error message to client: " << std::strerror(errno) << "\n";
            ERR_print_errors_fp(stderr);
        }
        return;
    }

    // Send success response
    std::string successMsg = "SUCCESS";
    if (SSL_write(ssl, successMsg.c_str(), successMsg.size()) <= 0) {
        std::cerr << "Failed to send success message: " << std::strerror(errno) << "\n";
        ERR_print_errors_fp(stderr);
        return;
    }

    std::cout << "File successfully deleted: " << filepath << "\n";
}