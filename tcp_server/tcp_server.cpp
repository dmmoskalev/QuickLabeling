#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX 17
#define PORT 5001
#define SA struct sockaddr
   
// Function designed for chat between client and server.
void func(int connfd)
{
    unsigned char buff[MAX];
    unsigned char returned_buff[13] = {0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0xAE, 0x00, 0x00, 0x00, 0x01, 0x03};
    // infinite loop for chat
    for (;;) {
        bzero(buff, MAX);
   
        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        /*std::cout << "From client: ";
        for (int i = 0; i < MAX; ++i)
        {
           printf(" %#x", buff[i] );
        }
        std::cout << std::endl;*/
        bzero(buff, MAX);
                  
        // and send that buffer to client
        write(connfd, returned_buff, sizeof(returned_buff));
           
    }
}
   
// Driver function
int main(int argc, char **argv) 
{
    int sockfd, connfd; 
    unsigned int len;
    struct sockaddr_in servaddr, cli;
    char  opt;
    int testmode = 0;
    if ((opt = getopt(argc, argv, "t")) != -1){
        	if (opt=='t') testmode = 1;
    }
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("SRV: socket creation failed...\n");
        return -1;
    }
    else
        printf("SRV: Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("SRV: socket bind failed...\n");
        return -1;
    }
    else
        printf("SRV: Socket successfully binded..\n");
   
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("SRV: Listen failed...\n");
        return -1;
    }
    else
        printf("SRV: Server is ready..\n");
    len = sizeof(cli);
    if (testmode == 0) {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("SRV: server accept failed...\n");
            return -1;
        }
        else
            printf("SRV: server accept the client...\n");
    
        // Function for chatting between client and server
        func(connfd);
    }
    // After chatting close the socket
    close(sockfd);
    return 0;
}
