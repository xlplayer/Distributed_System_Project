#include "Epoll.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <queue>

using namespace rapidjson;
using std::shared_ptr;
using std::queue;
using std::string;

const int LISTENQ = 5;
const int BUFFSIZE = 1024;
char buf[BUFFSIZE];
queue<string> MQ;
const int MAXQUEUESIZE = 102400;

int socket_bind(int port);
void handle_accept(shared_ptr<Epoll> _epoll, int listenfd);
void handle_read(shared_ptr<Epoll> _epoll, int fd);

int main(int argc, char **argv)
{
    int port = 80;
    int threadsNum = 4;
    int opt;
    const char *str = "t:l:p:";
    while((opt = getopt(argc, argv, str)) != -1)
    {
        switch(opt)
        {
            case 't':
            {
                threadsNum = atoi(optarg);
                break;
            }
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
    shared_ptr<Epoll> _epoll(new Epoll());
    _epoll->add_event(listenfd, EPOLLIN);

    while(1)
    {
        _epoll->wait_event();
        for(int i=0; i<_epoll->_eventNum; i++)
        {
            int fd = _epoll->_events[i].data.fd;
            if(fd == listenfd &&_epoll->_events[i].events & EPOLLIN)
                handle_accept(_epoll, listenfd);
            else if(fd != listenfd &&_epoll->_events[i].events & EPOLLIN)
                handle_read(_epoll, fd);
        }
    }
}

void handle_read(shared_ptr<Epoll> _epoll, int fd)
{
    int nread = read(fd, buf, BUFFSIZE);
    if(nread == -1)
    {
        perror("read error");
        close(fd);
        _epoll->del_event(fd, EPOLLIN);
    }
    else if(nread == 0)
    {
        fprintf(stderr,"client close.\n");
        close(fd);
        _epoll->del_event(fd, EPOLLIN);
    }
    else
    {
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(buf[i]);
        printf("\n");
        
        Document d;
        d.Parse(buf, nread);
        string type = d["type"].GetString();
        if(type == "push")
        {
            if(MQ.size() >= MAXQUEUESIZE)
            {
                write(fd, "push failure", 12);
            }
            else
            {
                MQ.push(string(buf, buf+nread));
                write(fd, "push success", 12);
            }  
        }
        else if(type == "pop")
        {
            if(MQ.empty())
            {
                write(fd, "pop failure", 11);
            }
            else
            {
                string msg = MQ.front();
                MQ.pop();
                write(fd, msg.c_str(), msg.length());
            }
        }
    }
}

void handle_accept(shared_ptr<Epoll> _epoll,int listenfd)
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
    if(clientfd == -1)
    {
        perror("accept error");
        exit(1);
    }
    printf("new connection from %s:%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    _epoll->add_event(clientfd, EPOLLIN);
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
