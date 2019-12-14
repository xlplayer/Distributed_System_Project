#include "MessageProducer.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <queue>
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/writer.h"
#include "MessageQueue.h"
#include "Channel.h"
#include <memory>
#include <unistd.h>
#include <sys/epoll.h>

using std::shared_ptr;
using namespace rapidjson;
using std::queue;

extern MessageQueue msgQueue;

MessageProducer::MessageProducer()
{
    initQueueServerAddr();
    int index = rand()%_queueServerAddr.size();

    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(_sockfd, (struct sockaddr*)&_queueServerAddr[index], sizeof(_queueServerAddr[index])) == -1)
    {
        perror("producer connect failed.");
    }
}

MessageProducer::~MessageProducer()
{
    close(_sockfd);
}


void MessageProducer::initQueueServerAddr()
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(50000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    _queueServerAddr.push_back(servaddr);
}

void MessageProducer::loop()
{
    char buf[1024];
    while(1)
    {
        Message msg = msgQueue.dequeue();
        shared_ptr<Channel> channel = msg.channel;
        string str = msg.msg;
        int nwrite = write(_sockfd, str.c_str(), str.length());
        int nread = read(_sockfd, buf, 1024);
        str = string(buf, buf+nread);
        if(str == "{\"result\":\"push failure\"}")
        {
            StringBuffer s;
            Writer<StringBuffer> writer(s);
            writer.StartObject();
            writer.Key("result");writer.String("failure");
            writer.EndObject();

            channel->setWritemsg(s.GetString());
            channel->setEvents(EPOLLOUT);
        }
    }
}
