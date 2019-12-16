#include "Acceptor.h"
#include "Channel.h"
#include "Epoll.h"
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "base/Socket.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
using std::make_shared;
using std::bind;

extern const int MAXFDS;
const int LISTENQ = 100;

Acceptor::Acceptor(int port, int threadsNum)
:_listenfd(socket_bind(port))
,_epoll(new Epoll())
,_eventLoopThreadPool(new EventLoopThreadPool(threadsNum))
{
    listen(_listenfd, LISTENQ);
    _eventLoopThreadPool->start();
}

Acceptor::~Acceptor()
{
    close(_listenfd);
}

void Acceptor::loop()
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    while(1)
    {
        clientfd = accept(_listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
        #ifdef DEBUG 
        printf("new connection from %s:%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        #endif
        if(clientfd >= MAXFDS)
        {
            close(clientfd);
            continue;
        }

        shared_ptr<EventLoopThread> thread = _eventLoopThreadPool->getNextEventLoopThread();
        shared_ptr<EventLoop> loop = thread->getEventLoop();
        shared_ptr<Epoll> epoll = loop->getEpoll();

        setSocketNonBlocking(clientfd);
        shared_ptr<Channel> channel(new Channel(loop, clientfd));
        channel->setEvents(EPOLLIN );
        loop->addPendingFunctions(bind(&Epoll::add_channel, epoll, channel));
        loop->wakeup();
        //epoll->add_channel(channel);
    }
}
