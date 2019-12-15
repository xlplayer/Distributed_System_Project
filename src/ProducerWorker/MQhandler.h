#ifndef MQHADLER_H
#define MQHADLER_H

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
using std::string;
using std::vector;

class MQhandler
{
    public:
        MQhandler();
        ~MQhandler();
        void initQueueServerAddr();
        void loop();

    private:
        vector<int> _sockfd;
        int _listenfd;
        string _listen_ip;
        int _listen_port;
};

#endif