#include "Thread.h"
#include "Condition.h"
#include "MutexLock.h"

Thread::Thread(function<void()> &&cb)
:_isRunning(false)
,_tid(0)
,_cb(cb)
,_mutex()
,_cond(_mutex)
{}

Thread::~Thread()
{
    if(_isRunning)
    {
        pthread_detach(_tid);
    }
}

struct threadData //use this to prevent new thread run after data has been release
{
    threadData(bool *started, MutexLock &mutex, Condition &cond, function<void()> &func)
    :_started(started),
    _mutex(mutex),
    _cond(cond),
    _func(func)
    {}
    bool *_started;
    MutexLock &_mutex;
    Condition &_cond;
    function<void()> &_func;
    void run()
    {
        {
            MutexLockGuard lock(_mutex);
            *_started = true;
            _cond.notifyAll();
        }
        _func();
    }
};

void* Thread::threadFunc(void *arg)
{
    threadData *data = static_cast<threadData*>(arg);
    data->run();
    delete data;
    return NULL;
}

void Thread::start()
{
    bool started = false;
    threadData *data = new threadData(&started, _mutex,_cond, _cb);
    pthread_create(&_tid, NULL, threadFunc, data);
    {
        MutexLockGuard lock(_mutex);
        while(started == false)
            _cond.wait();
    }
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