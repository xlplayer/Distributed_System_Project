#include "MessageConsumer.h"
#include <unistd.h>
#include <arpa/inet.h>
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/writer.h"
using namespace rapidjson;

MessageConsumer::MessageConsumer(vector<sockaddr_in> &queueServerAddr)
:_queueServerAddr(queueServerAddr)
,_num(queueServerAddr.size())
{
    int index = rand()%_num;

    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(_sockfd, (struct sockaddr*)&_queueServerAddr[index], sizeof(_queueServerAddr[index])) == -1)
    {
        perror("consumer connect failed.");
    }
    
}

MessageConsumer::~MessageConsumer()
{
    close(_sockfd);
}

string MessageConsumer::pop()
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("type");
    writer.String("pop");
    writer.EndObject();

    write(_sockfd, s.GetString(), s.GetSize());
    char buf[1024];
    int n = read(_sockfd, buf, 1024);
    return string(buf, buf+n);
}
