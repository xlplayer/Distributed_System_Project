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
    threadData(Condition &cond, function<void()> &func):_cond(cond),_func(func){}
    Condition &_cond;
    function<void()> &_func;
    void run()
    {
        _cond.notifyAll();
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
    threadData *data = new threadData(_cond, _cb);
    pthread_create(&_tid, NULL, threadFunc, data);
    _cond.wait();
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