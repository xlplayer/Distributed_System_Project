#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int MAXLINE = 1024;

int main(int argc, char **argv)
{
    if(argc != 3) 
    {
        perror("usage: ./client <ipaddress> <port>");
        exit(1);
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    char sendline[MAXLINE],recvline[MAXLINE];
    while(fgets(sendline,MAXLINE,stdin))
    {
        sendline[strlen(sendline)-1] = 0;
        write(sockfd, sendline, strlen(sendline));
        int nread = read(sockfd, recvline, MAXLINE);
        for(int i=0;i<nread;i++) putchar(recvline[i]);
        printf("\n");
        nread = read(sockfd, recvline, MAXLINE);
        for(int i=0;i<nread;i++) putchar(recvline[i]);
        printf("\n");
    }
}