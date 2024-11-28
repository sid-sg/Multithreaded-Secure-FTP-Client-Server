#include <iostream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include "./include/progressbar.hpp"

#define PORT 7000
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 4096

int main(){

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  //server fd
    if(sockfd == -1){
        std::cerr<< "socket creation failed: "<<std::strerror(errno)<<"\n";
        return 1;
    }
    std::cout<<"Socket created\n";

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);



    if( connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1){
        std::cerr<< "failed connect(): "<<std::strerror(errno)<<"\n";
    }
    std::cout<<"server connected\n";
        
    // char buffer[BUFFER_SIZE];

    //send name
    std::cout<<"Enter your name: \n";
    std::string buffer;
    std::string name; 
    std::cin>>buffer;
    if(buffer.length()==0){
        std::cerr<< "enter valid name"<<"\n";
        close(sockfd);
        return 1;
    }

    // std::cout<<buffer<<"\n";

    int bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

    if( bytesSent == -1){
        std::cerr<< "failed sending: "<<std::strerror(errno)<<"\n";
        close(sockfd);
        return 1;
    }

    name = buffer;

    //send option

    int option = 0;

    while(1){

        std::cout<<"Choose\n 1. ls /"<<name<<" directory on server \t 2. upload file: \n";
        std::cin>>option;
        buffer.clear();

        if(option == 1){ //ls
            //send ls
            buffer = "ls";
            bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

            if( bytesSent == -1){
                std::cerr<< "failed sending: "<<std::strerror(errno)<<"\n";
                close(sockfd);
                return 1;
            }
            buffer.clear();


            char recvBuffer[BUFFER_SIZE];
            int bytesRecv = recv(sockfd, recvBuffer, sizeof(recvBuffer) - 1, 0);
            if (bytesRecv <= 0) {
                std::cerr << "Failed receiving: " << std::strerror(errno) << "\n";
            }
            
            recvBuffer[bytesRecv] = '\0';
            std::cout << "Directory files: \n" << recvBuffer << "\n";


        }
        else if(option == 2){ //upload

            // send upload
            buffer = "upload";
            bytesSent = send(sockfd, buffer.c_str(), buffer.length(), 0);

            if( bytesSent == -1){
                std::cerr<< "failed sending upload request: "<<std::strerror(errno)<<"\n";
                // close(sockfd);
                continue;
            }
            buffer.clear();

            std::string pathname;    
            std::cout<<"Enter file pathname: \n";
            std::cin>>pathname;

            int filefd = open(pathname.c_str(), O_RDONLY);
            if(filefd==-1){
                std::cerr<< "failed opening file: "<<std::strerror(errno)<<"\n";
                close(filefd);
                continue;
            }

            std::string filename = pathname.substr(pathname.find_last_of("/")+1);
            
            off_t filesize = lseek(filefd, 0, SEEK_END);
            lseek(filefd, 0, SEEK_SET);

            std::string metadata = filename + ":" + std::to_string(filesize);

            bytesSent = send(sockfd, metadata.c_str(), metadata.length(), 0);

            if( bytesSent == -1){
                std::cerr<< "failed sending metadata: "<<std::strerror(errno)<<"\n";
                continue;
            }

            char ACK[3];
            
            int bytesRecv = recv(sockfd, ACK, sizeof(ACK)-1, 0);

            ACK[bytesRecv] = '\0';

            if( bytesRecv == -1){
                std::cerr<< "failed recving metadata ACK: "<<std::strerror(errno)<<"\n";
                continue;
            }

            std::cout<<"ACK: "<<ACK<<"\n";

            if( strcmp(ACK, "OK") !=0 ){
                std::cerr<< "recieved NACK: "<<"\n";
                continue;
            }

            int totalSteps = 100;
            ProgressBar progressBar(totalSteps);
            off_t totalBytesSent = 0; 
            off_t bytesRemaining = filesize; 
            const size_t chunkSize = 4096; 

            while (bytesRemaining>0) {
                size_t remaining = std::min(chunkSize, static_cast<size_t>(bytesRemaining) );
                bytesSent = sendfile(sockfd, filefd, &totalBytesSent, remaining);
                if (bytesSent == -1) {
                    std::cerr << "failed sending file: " << std::strerror(errno) << "\n";
                    close(filefd);
                    break;
                }

                bytesRemaining -= bytesSent;

                int progress = static_cast<int>( (static_cast<float>(totalBytesSent) / filesize )* 100 );
                progress = std::min(progress, totalSteps);                
                progressBar.update(progress);
            }

            std::cout<<"\n";

            if( bytesSent == -1){
                std::cerr<< "failed sending file: "<<std::strerror(errno)<<"\n";
                close(filefd);
                continue;
            }

            std::cout<<"File sent\n";


            buffer.clear();
        }
        else{
            std::cerr<< "invalid option"<<"\n";
            close(sockfd);
            return 1;
            // continue;
        }
    }

    

    close(sockfd);

    return 0;
}