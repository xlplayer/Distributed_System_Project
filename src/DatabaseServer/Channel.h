#ifndef CHANNEL_H
#define CHANNEL_H

#include <memory>
#include <string.h>
#include <queue>
#include <functional>
using std::enable_shared_from_this;
using std::shared_ptr;
using std::function;
using std::string;
using std::queue;

enum STATE 
{
    CONNECTIING,
    DISCONNTING,
    DISCONNTED
};
class EventLoop;
class Epoll;
class Database;

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
        void setWritemsg(string msg) { _writemsg = msg; }
        void handleRead();
        void handleWrite();
        void handleEvents();
    
    private:
        
        shared_ptr<EventLoop> _eventLoop;
        shared_ptr<Epoll> _epoll;
        shared_ptr<Database> _database;
        queue<string> &_msgQueue;
        int _fd;
        string _readmsg, _writemsg;
        int _curlyCount;
        uint32_t _events;
        uint32_t _revents;
        STATE _state;
        

        function<void()> _readHandler;
        function<void()> _writerHandler;
};
#endif