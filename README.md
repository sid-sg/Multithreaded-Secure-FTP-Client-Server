# Preview
https://github.com/user-attachments/assets/e8572f6c-48f4-42b0-9bf0-2ab010d9eace

# Dependencies
## OpenSSL: For Secure FTP using SSL/TLS protocol and PBKDF2 hashing
`sudo apt-get install openssl libssl-dev`
## Zlib: For Gzip compression/decompression
`sudo apt install zlib1g zlib1g-dev`
## SQLite3: For storing user info
`sudo apt install sqlite3 libsqlite3-dev`

# Generate SSL certificate and key for server
## Go to /security
`cd server/security`
## Generate private key
`openssl genrsa -out server.key 2048`
## Create a CSR (Certificate Signing Request)
`openssl req -new -key server.key -out server.csr`
## Generate self-signed certificate
`openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt`

# Build
## Build server
```
cd server
mkdir -p build
cd build
cmake ..
cmake --build .
sudo ./bin/server <server_portno>
```
example: ``sudo ./bin/server 8000``
## Build client
```
cd client
mkdir -p build
cd build
cmake ..
cmake --build .
./bin/client <server_ipaddress> <server_portno>
```
example: ``./bin/server 127.0.0.1 8000``

# Debug using valgrind
## server
``sudo valgrind --leak-check=full --track-origins=yes ./bin/server <server_portno>``
## client
``valgrind --leak-check=full --track-origins=yes ./bin/client <server_ipaddress> <server_portno>``
