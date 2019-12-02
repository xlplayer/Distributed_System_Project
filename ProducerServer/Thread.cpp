#include "Thread.h"

Thread::Thread(function<void()> cb)
:_isRunning(false)
,_tid(0)
,_cb(cb)
{}

Thread::~Thread()
{
    if(_isRunning)
    {
        pthread_detach(_tid);
    }
}

void* Thread::threadFunc(void *arg)
{
    Thread *p = static_cast<Thread*>(arg);
    if(p) p->_cb();
    return NULL;
}

void Thread::start()
{
    pthread_create(&_tid, NULL, threadFunc, this);
    _isRunning = true;
}

void Thread::join()
{
    if(_isRunning)
    {
        pthread_join(_tid, NULL);
        _isRunning = false;
    }
}