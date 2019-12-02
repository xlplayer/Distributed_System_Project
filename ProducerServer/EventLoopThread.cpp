#include "EventLoopThread.h"
#include "EventLoop.h"
#include "Thread.h"

EventLoopThread::EventLoopThread()
:_eventLoop(new EventLoop())
,_thread(new Thread(std::bind(&EventLoopThread::threadFunc,this)))
{}

EventLoopThread::~EventLoopThread()
{
    _thread->join();
}

void EventLoopThread::startLoop()
{
    _thread->start();
}

void EventLoopThread::threadFunc()
{
    _eventLoop->loop();
}