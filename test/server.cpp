#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int LISTENQ = 5;
const int EPOLLEVENTS = 100;
const int MAXSIZE = 1024;
const int FDSIZE = 1000;

int socket_bind(int port);
void do_epoll(int listenfd);
void handle_accept(int epollfd,int listenfd);
void do_read(int epollfd,int fd,char *buf);
void add_event(int epollfd,int fd,int state);
void mod_event(int epollfd,int fd,int state);
void del_event(int epollfd,int fd,int state);

int main(int argc, char **argv)
{
    int port = 80;
    int opt;
    const char *str = "t:l:p:";
    while((opt = getopt(argc, argv, str)) != -1)
    {
        switch(opt)
        {
            case 'p':
            {
                port = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
    int listenfd = socket_bind(port);
    listen(listenfd, LISTENQ);
    do_epoll(listenfd);
}

int socket_bind(int port)
{
    if(port < 0|| port > 65535) return -1;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind error");
        exit(1);
    }
    return listenfd;
}

void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void del_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void mod_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);    
}

void handle_accept(int epollfd, int listenfd)
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len;
    clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
    if(clientfd == -1)
    {
        perror("accept error");
        exit(1);
    }
    printf("new connection from %s:%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    add_event(epollfd, clientfd, EPOLLIN);
}

void do_read(int epollfd, int fd, char *buf)
{
    int nread = read(fd, buf, MAXSIZE);
    if(nread == -1)
    {
        perror("read error");
        close(fd);
        del_event(epollfd, fd, EPOLLIN);
    }
    else if(nread == 0)
    {
        fprintf(stderr,"client close.\n");
        close(fd);
        del_event(epollfd, fd, EPOLLIN);
    }
    else
    {
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(buf[i]);
        printf("\n");
        int nwrite = write(fd, buf, nread);
        if(nwrite == -1)
        {
            perror("write error");
            exit(1);
        }
    }
    
}

void do_epoll(int listenfd)
{
    int epollfd = epoll_create(FDSIZE);
    struct epoll_event events[EPOLLEVENTS];
    int num;
    char buf[MAXSIZE];
    add_event(epollfd, listenfd, EPOLLIN);
    while(1)
    {
        num = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        for(int i = 0; i < num; i++)
        {
            int fd = events[i].data.fd;
            if(fd == listenfd && (events[i].events & EPOLLIN)) //new connection
                handle_accept(epollfd, listenfd);
            else if(events[i].events & EPOLLIN)
                do_read(epollfd, fd, buf);
        }
    }
}