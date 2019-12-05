#ifndef MESSAGEPRODUCER_H
#define MESSAGEPRODUCER_H

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
using std::string;
using std::vector;

class MessageProducer
{
    public:
        MessageProducer();
        ~MessageProducer();
        void initQueueServerAddr();
        void loop();

    private:
        int _sockfd;
        int _listenfd;
        string _listen_ip;
        int _listen_port;
        vector<sockaddr_in> _queueServerAddr;
};

#endif