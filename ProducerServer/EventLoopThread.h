#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <memory>
#include <vector>
#include <netinet/in.h>

using std::vector;
using std::shared_ptr;

class EventLoop;
class Thread;

class EventLoopThread
{
    public:
        EventLoopThread();
        ~EventLoopThread();
        void startLoop();
        shared_ptr<EventLoop> getEventLoop(){
            return _eventLoop;
        }

    private:
        void threadFunc();
        shared_ptr<Thread> _thread;
        shared_ptr<EventLoop> _eventLoop;

};

#endif