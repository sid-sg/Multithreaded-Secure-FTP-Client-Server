# Preview
https://github.com/user-attachments/assets/e8572f6c-48f4-42b0-9bf0-2ab010d9eace

---

## Dependencies
Ensure the following libraries are installed:

### OpenSSL (For Secure FTP using SSL/TLS protocol and PBKDF2 hashing)
```bash
sudo apt-get install openssl libssl-dev
```

### Zlib (For Gzip compression/decompression)
```bash
sudo apt install zlib1g zlib1g-dev
```

### SQLite3 (For storing user information)
```bash
sudo apt install sqlite3 libsqlite3-dev
```

---

## Generating SSL Certificate and Key for the Server

1. Navigate to the `/security` directory inside the server folder:
   ```bash
   cd server/security
   ```

2. Generate a self-signed SSL certificate and private key:
   ```bash
   openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
   -keyout server.key -out server.crt -config openssl.cnf
   ```

3. Copy the generated certificate (`server.crt`) to the client’s `/security` directory:
   ```bash
   cp server.crt ../../client/security
   ```

---

## Build Instructions

### Build the Server
1. Navigate to the server directory:
   ```bash
   cd server
   ```

2. Create a build directory and compile:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   cmake --build .
   ```

3. Run the server:
   ```bash
   sudo ./bin/server <server_portno>
   ```
   **Example:** 
   ```bash
   sudo ./bin/server 8000
   ```

### Build the Client
1. Navigate to the client directory:
   ```bash
   cd client
   ```

2. Create a build directory and compile:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   cmake --build .
   ```

3. Run the client:
   ```bash
   ./bin/client <server_ipaddress> <server_portno>
   ```
   **Example:**
   ```bash
   ./bin/client 127.0.0.1 8000
   ```

---

## Debugging with Valgrind

Valgrind can be used to check memory leaks and runtime errors.

### Debugging the Server
```bash
sudo valgrind --leak-check=full --track-origins=yes ./bin/server <server_portno>
```

### Debugging the Client
```bash
valgrind --leak-check=full --track-origins=yes ./bin/client <server_ipaddress> <server_portno>
```

---

## Example Usage
### Start the Server
```bash
sudo ./bin/server 8000
```

### Start the Client
```bash
./bin/client 127.0.0.1 8000
