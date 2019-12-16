#include "Socket.h"
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
using std::string;

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

int readn(int fd, string &msg, bool &zero)
{
    char buf[1024];
    int n=0, sum=0;
    while(1)
    {
        if((n = read(fd, buf, 1024)) < 0)
        {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN)
            {
                return sum;
            }  
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if(n == 0)
        {
            zero = true;
            break; /*EOF*/
        }
        sum += n;
        msg += string(buf, buf+n);
    }
    return sum;
}

int writen(int fd, string &msg)
{
    int left=msg.size();
    int n=0, sum=0;
    const char *p = msg.c_str();
    while(left > 0)
    {
        if ((n = write(fd, p, left)) < 0)
        {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN)
                return sum;
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if(n == 0)
        {
            break;
        }
        sum += n;
        left -= n;
        p += n;
    }
    msg = msg.substr(sum);
    return sum;
}

int writen(int fd, void* buf, int size) //for eventfd
{
    int left = size;
    int n=0, sum=0;
    char *ptr = (char *) buf;
    while(left > 0)
    {
        if((n = write(fd, ptr, left)) <= 0)
        {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN)
                return sum;
            else
                return -1; 
        }
        sum += n;
        left -=n;
        ptr += n;
    }
    return sum;
}

void setSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        perror("get socket flag failed");
    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        perror("set socket non-block failed");
}
