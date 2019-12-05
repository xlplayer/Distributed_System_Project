#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <functional>
using std::function;

class Thread
{
    public:
        Thread(function<void()> &&cb);
        ~Thread();
        void start();
        void join();
        static void *threadFunc(void *arg);
    
    private:
        bool _isRunning;
        pthread_t _tid;
        function<void()> _cb;
};

#endif