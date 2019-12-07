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
#include "MutexLock.h"
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
        EventLoop(vector<sockaddr_in> &queueServerAddr, vector<sockaddr_in> &_databaseServerAddr);
        ~EventLoop();
        void handleQueueRead();
        void handleQueueWrite();
        void handleDatabaseRead();
        void handleDatabaseWrite();
        void handlewakeup();
        void handleConnect();
        void loop();
        shared_ptr<Epoll> getEpoll(){ return _epoll; }
        void addPendingFunctions(function<void()> &&cb);
        void doPendingFunctions();
        void wakeup();
   
    private:
        map<pair<string, int>, shared_ptr<Channel> > _addr2channel;
        queue<string> _msgQueue;
        vector<sockaddr_in> &_queueServerAddr, &_databaseServerAddr;
        int _queuefd;
        int _databasefd;
        int _wakeupfd;
        shared_ptr<Channel> _wakeupChannel;
        shared_ptr<Channel> _queueChannel;
        shared_ptr<Channel> _databaseChannel;
        int _curlyCount;
        shared_ptr<Epoll> _epoll;

        vector<function<void()>> _pendingFunctions;
        MutexLock _mutex;
};

#endif