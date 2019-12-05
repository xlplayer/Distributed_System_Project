#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <string>
#include <functional>
#include "MutexLock.h"
#include "MessageProducer.h"
using std::shared_ptr;
using std::string;
using std::function;
using std::enable_shared_from_this;

class Epoll;
class Channel;

class EventLoop: public enable_shared_from_this<EventLoop>
{
    public:
        EventLoop();
        ~EventLoop();
        void handlewakeup();
        void handleConnect();
        void loop();
        string getListenip() { return _listen_ip; }
        int getListenport() { return _listen_port; }
        shared_ptr<Epoll> getEpoll(){ return _epoll; }
        void addPendingFunctions(function<void()> &&cb);
        void doPendingFunctions();
        void wakeup();
   
    private:
        int _wakeupfd;
        shared_ptr<Channel> _wakeupChannel;
        shared_ptr<Epoll> _epoll;
        shared_ptr<Channel> _acceptChannel;
        int _listenfd;
        string _listen_ip;
        int _listen_port;
        vector<function<void()>> _pendingFunctions;
        MutexLock _mutex;
};

#endif