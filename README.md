# Preview
https://github.com/user-attachments/assets/e8572f6c-48f4-42b0-9bf0-2ab010d9eace

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
``valgrind --leak-check=full --track-origins=yes ./bin/server <server_portno>``
## client
``valgrind --leak-check=full --track-origins=yes ./bin/client <server_ipaddress> <server_portno>``
