#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "Epoll.h"
#include "Acceptor.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
using std::shared_ptr;

vector<sockaddr_in> queueServerAddr;
void initQueueServerAddr(vector<sockaddr_in> &queueServerAddr)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(50000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    queueServerAddr.push_back(servaddr);
}

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

    initQueueServerAddr(queueServerAddr);
    shared_ptr<Acceptor> acceptor(new Acceptor(port, threadsNum));
    acceptor->loop();
}