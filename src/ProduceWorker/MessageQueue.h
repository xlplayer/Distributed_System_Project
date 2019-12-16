#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <string>
#include <queue>
#include <memory>
#include "base/MutexLock.h"
#include "base/Condition.h"

using std::queue;
using std::string;
using std::shared_ptr;

class Channel;

struct Message
{
    Message(shared_ptr<Channel> c, string m):channel(c),msg(m){}
    string msg;
    shared_ptr<Channel> channel;
};

class MessageQueue
{
    public:
        MessageQueue();
        ~MessageQueue();
        int getSize(){ return _Queue.size(); }
        void enqueue(Message msg);
        Message dequeue();
    
    private:
        MutexLock _mutex;
        Condition _cond;
        queue<Message> _Queue;

};

#endif