#ifndef MESSAGEConsumer_H
#define MESSAGEConsumer_H

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
using std::string;
using std::vector;

class MessageConsumer
{
    public:
        MessageConsumer(vector<sockaddr_in> &queueServerAddr);
        ~MessageConsumer();
        string pop();

    private:
        vector<sockaddr_in> &_queueServerAddr;
        int _num;
        int _sockfd;
};

#endif