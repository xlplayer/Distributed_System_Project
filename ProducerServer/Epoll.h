#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <memory>
using std::vector;
using std::shared_ptr;

const int MAXEVENTS = 100;
class Channel;

class Epoll
{
    public:
        Epoll();
        ~Epoll();
        void add_event(int fd, int state);
        void mod_event(int fd, int state);
        void del_event(int fd, int state);
        void add_channel(shared_ptr<Channel> channel);
        void mod_channel(shared_ptr<Channel> channel);
        void del_channel(shared_ptr<Channel> channel);
        shared_ptr<Channel> get_channel(int fd){ return _fd2channel[fd]; }
        void handle_activate_channels();
    
    private:
        static const int MAXFDS = 102400; 
        vector<epoll_event> _events;
        int _epollfd;
        shared_ptr<Channel> _fd2channel[MAXFDS];   
};

#endif