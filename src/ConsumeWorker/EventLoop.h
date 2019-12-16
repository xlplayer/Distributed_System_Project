#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <functional>
#include "base/MutexLock.h"
using std::pair;
using std::shared_ptr;
using std::map;
using std::vector;
using std::queue;
using std::string;
using std::function;
using std::enable_shared_from_this;

class Epoll;
class Channel;

class EventLoop: public enable_shared_from_this<EventLoop>
{
    public:
        EventLoop();
        ~EventLoop();
        void handleQueueRead(shared_ptr<Channel> queueChannel, int queuefd);
        void handleQueueWrite(shared_ptr<Channel> queueChannel, int queuefd);
        void handleDatabaseRead(shared_ptr<Channel> databaseChannel, int databasefd);
        void handleDatabaseWrite(shared_ptr<Channel> databaseChannel, int databasefd);
        void handlewakeup();
        void handleConnect();
        void loop();
        shared_ptr<Epoll> getEpoll(){ return _epoll; }
        map<pair<string, int>, shared_ptr<Channel> > & getAddr2Channel(){ return _addr2channel;}
        void addPendingFunctions(function<void()> &&cb);
        void doPendingFunctions();
        void wakeup();
   
    private:
        map<pair<string, int>, shared_ptr<Channel> > _addr2channel;
        queue<string> _msgQueue;
        //vector<sockaddr_in> &_queueServerAddr, &_databaseServerAddr;
        vector<int> _queuefds;
        vector<int> _databasefds;
        int _wakeupfd;
        shared_ptr<Channel> _wakeupChannel;
        vector<shared_ptr<Channel> > _queueChannels;
        vector<shared_ptr<Channel> >_databaseChannels;
        int _curlyCount;
        shared_ptr<Epoll> _epoll;

        vector<function<void()>> _pendingFunctions;
        MutexLock _mutex;
};

#endif