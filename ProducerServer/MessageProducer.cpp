#include "MessageProducer.h"
#include <unistd.h>
#include <arpa/inet.h>
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/writer.h"
using namespace rapidjson;

extern vector<sockaddr_in> queueServerAddr;

MessageProducer::MessageProducer()
{
    int index = rand()%queueServerAddr.size();

    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(_sockfd, (struct sockaddr*)&queueServerAddr[index], sizeof(queueServerAddr[index])) == -1)
    {
        perror("producer connect failed.");
    }
}

MessageProducer::~MessageProducer()
{
    close(_sockfd);
}

string MessageProducer::push(string &ip, int port, int fd, string &msg)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("type");
    writer.String("push");
    writer.Key("ip");
    writer.String(ip.c_str());
    writer.Key("port");
    writer.Int(port);
    writer.Key("fd");
    writer.Int(fd);
    writer.Key("message");
    writer.String(msg.c_str());
    writer.EndObject();
    write(_sockfd, s.GetString(), s.GetSize());

    char buf[1024];
    int n = read(_sockfd, buf, 1024);
    return string(buf, buf+n);
}
