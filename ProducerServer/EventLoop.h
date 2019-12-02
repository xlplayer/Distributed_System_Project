#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <string>
#include "MessageProducer.h"
using std::shared_ptr;
using std::string;

class Epoll;
const int BUFFSIZE = 1024;

class EventLoop
{
    public:
        EventLoop();
        ~EventLoop();
        void handle_read(int fd);
        void hadnle_accept(int fd);
        void loop();
        void addToEpoll(int fd);
   
    private:
        shared_ptr<Epoll> _epoll;
        char _buf[BUFFSIZE];
        shared_ptr<MessageProducer> _messageProducer;
        int _listenfd;
        string _listen_ip;
        int _listen_port;
};

#endif