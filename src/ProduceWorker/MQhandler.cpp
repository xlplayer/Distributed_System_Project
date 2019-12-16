#include "MQhandler.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <queue>
#include "MessageQueue.h"
#include "Channel.h"
#include <memory>
#include <unistd.h>
#include <sys/epoll.h>

using std::shared_ptr;
using std::queue;

extern vector<sockaddr_in> queueServerAddr;
 
extern MessageQueue msgQueue;

MQhandler::MQhandler()
{
    for(int index = 0; index<queueServerAddr.size(); index++)
    {
        for(int i = 0;i < 4; i++)
        {
            int fd = socket(AF_INET, SOCK_STREAM, 0);
            if(connect(fd, (struct sockaddr*)&queueServerAddr[index], sizeof(queueServerAddr[index])) == -1)
            {
                perror("producer connect failed.");
            }
            else
                _sockfd.push_back(fd);
        }
    }
}

MQhandler::~MQhandler()
{
    for(int i=0;i<_sockfd.size();i++)
        close(_sockfd[i]);
}

void MQhandler::loop()
{
    char buf[1024];
    while(1)
    {
        int index = rand()%_sockfd.size();
        int fd = _sockfd[index];

        Message msg = msgQueue.dequeue();
        shared_ptr<Channel> channel = msg.channel;
        string str = msg.msg;
        int nwrite = write(fd, str.c_str(), str.length());
        int nread = read(fd, buf, 1024);
        str = string(buf, buf+nread);
        if(str == "{\"result\":\"push failure\"}")
        {
            channel->setWritemsg("{\"result\":\"failure\"}");
            channel->setEvents(EPOLLOUT );
        }
    }
}
