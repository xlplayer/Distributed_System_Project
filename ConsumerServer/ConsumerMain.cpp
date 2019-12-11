#include "Thread.h"
#include "EventLoopThreadPool.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <iostream>
using namespace std;
using namespace rapidjson;
using std::shared_ptr;
using std::vector;

vector<sockaddr_in> queueServerAddr,databaseServerAddr;
void initQueueServerAddr(vector<sockaddr_in> &queueServerAddr);
void initDatabaseServerAddr(vector<sockaddr_in> &databaseServerAddr);

int main(int argc, char **argv)
{
    initQueueServerAddr(queueServerAddr);
    initDatabaseServerAddr(databaseServerAddr);

    int threadsNum = 4;
    int opt;
    const char *str = "t:";
    while((opt = getopt(argc, argv, str)) != -1)
    {
        switch(opt)
        {
            case 't':
            {
                threadsNum = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
    shared_ptr<EventLoopThreadPool> _etp (new EventLoopThreadPool(threadsNum));
    _etp->start();
    while(1);
    // shared_ptr<EventLoop> eventLoop(new EventLoop(queueServerAddr,databaseServerAddr));
    // eventLoop->loop();
    //while(1);

}

void initQueueServerAddr(vector<sockaddr_in> &queueServerAddr)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(50000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    queueServerAddr.push_back(servaddr);
}

void initDatabaseServerAddr(vector<sockaddr_in> &databaseServerAddr)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(60000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    databaseServerAddr.push_back(servaddr);
}