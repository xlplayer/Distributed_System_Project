#include "MessageQueue.h"

MessageQueue::MessageQueue()
:_mutex()
,_cond(_mutex)
{}

MessageQueue::~MessageQueue()
{}

void MessageQueue::enqueue(Message msg)
{
    MutexLockGuard lock(_mutex);
    _Queue.push(msg);
    _cond.notifyAll();
}

Message MessageQueue::dequeue()
{
    MutexLockGuard lock(_mutex);
    while(_Queue.empty())
    {
        _cond.wait();
    }
    Message t = _Queue.front();
    _Queue.pop();
    return t;
}