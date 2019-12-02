#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <vector>
using std::vector;

const int MAXEVENTS = 100;

class Epoll
{
    public:
        Epoll();
        ~Epoll();
        void add_event(int fd, int state);
        void mod_event(int fd, int state);
        void del_event(int fd, int state);
        void wait_event();
        vector<epoll_event> _events;
        int _eventNum;
    
    private:
        int _epollfd;        
};

#endif