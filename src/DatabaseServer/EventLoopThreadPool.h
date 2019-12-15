#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "EventLoopThread.h"
#include <vector>
#include <memory>
#include <netinet/in.h>
#include <functional>
using std::vector;
using std::shared_ptr;

class EventLoopThreadPool
{
    public:
        EventLoopThreadPool(size_t threadNum);
        ~EventLoopThreadPool();
        void start();
        shared_ptr<EventLoopThread> getNextEventLoopThread();

    private:
        size_t _threadNum;
        size_t _next;
        vector<shared_ptr<EventLoopThread> > _eventLoopThreads;
};

#endif