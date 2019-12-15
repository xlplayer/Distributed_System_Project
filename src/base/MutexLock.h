#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H

#include <pthread.h>

class MutexLock
{
    public:
        MutexLock()
        {
            pthread_mutex_init(&_mutex, NULL);
        }
        ~MutexLock()
        {
            pthread_mutex_lock(&_mutex);
            pthread_mutex_destroy(&_mutex);
        }
        void lock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
        pthread_mutex_t* get()
        {
            return &_mutex;
        }
        
    private:
        pthread_mutex_t _mutex;
};


class MutexLockGuard
{
    public:
        MutexLockGuard(MutexLock &mutexLock)
        :_mutexLock(mutexLock)
        {
            _mutexLock.lock();
        }   
        ~MutexLockGuard()
        {
            _mutexLock.unlock();
        }

    private:
        MutexLock &_mutexLock;
};

#endif
