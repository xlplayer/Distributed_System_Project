#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <memory>
using std::shared_ptr;

class Epoll;
class EventLoopThreadPool;

class Acceptor
{
    public:
        Acceptor(int port, int threadsNum);
        ~Acceptor();
        void loop();
    
    private:
        int _listenfd;
        int _threadsNum;
        shared_ptr<Epoll> _epoll;
        shared_ptr<EventLoopThreadPool> _eventLoopThreadPool;
};

#endif