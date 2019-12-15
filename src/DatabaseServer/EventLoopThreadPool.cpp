#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(size_t threadNum)
:_threadNum(threadNum)
,_next(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::start()
{
    for(size_t i=0; i< _threadNum; i++)
    {
        shared_ptr<EventLoopThread> sp(new EventLoopThread());
        sp -> startLoop();
        _eventLoopThreads.push_back(sp);
    }
}

shared_ptr<EventLoopThread> EventLoopThreadPool::getNextEventLoopThread()
{
    shared_ptr<EventLoopThread> sp = _eventLoopThreads[_next];
    _next = (_next + 1) % _threadNum;
    return sp;
}