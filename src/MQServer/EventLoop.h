#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <queue>
#include "base/MutexLock.h"
using std::shared_ptr;
using std::vector;
using std::queue;
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
        shared_ptr<Epoll> getEpoll(){ return _epoll; }
        queue<string> &getQueue(){ return _msgQueue; }
        void addPendingFunctions(function<void()> &&cb);
        void doPendingFunctions();
        void wakeup();
   
    private:
        queue<string> _msgQueue;
        int _wakeupfd;
        shared_ptr<Channel> _wakeupChannel;
        shared_ptr<Epoll> _epoll;

        vector<function<void()>> _pendingFunctions;
        MutexLock _mutex;
};

#endif