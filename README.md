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
sudo ./bin/client <server_ipaddress> <server_portno>
```
example: ``sudo ./bin/server 127.0.0.1 8000``
