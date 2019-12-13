#ifndef CHANNEL_H
#define CHANNEL_H

#include <memory>
#include <string.h>
#include <functional>
#include <hiredis/hiredis.h>
using std::enable_shared_from_this;
using std::shared_ptr;
using std::function;
using std::string;

enum STATE 
{
    CONNECTIING,
    DISCONNTING,
    DISCONNTED
};
class EventLoop;
class Epoll;

class Channel: public enable_shared_from_this<Channel>
{
    public:
        Channel(shared_ptr<EventLoop> eventLoop, int fd);
        ~Channel();
        void setReadHandler(function<void()> &&readHandler){ _readHandler = readHandler; }
        void setWriterHandler(function<void()> &&writerHandler) { _writerHandler = writerHandler; }
        void setEvents(uint32_t events){ _events = events; }
        void update();
        uint32_t getEvents(){ return _events; }
        void setRevents(uint32_t revents){ _revents = revents; }
        int getFd(){ return _fd; }
        string getTime() { return _time; }
        string &getWritemsg() { return _writemsg; }
        void setWritemsg(string msg) { _writemsg = msg; }
        void handleRead();
        void handleWrite();
        void handleEvents();
        void addToEpoll();
    
    private:
        redisContext* _redis;
        shared_ptr<EventLoop> _eventLoop;
        shared_ptr<Epoll> _epoll;
        int _fd;
        string _readmsg, _writemsg;
        int _curlyCount;
        uint32_t _events;
        uint32_t _revents;
        STATE _state;
        string _time;

        function<void()> _readHandler;
        function<void()> _writerHandler;
};
#endif