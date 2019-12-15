#include "EventLoopThread.h"
#include "EventLoop.h"
#include "base/Thread.h"
using std::bind;

EventLoopThread::EventLoopThread()
:_eventLoop(new EventLoop())
,_thread(new Thread(bind(&EventLoopThread::threadFunc,this)))
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