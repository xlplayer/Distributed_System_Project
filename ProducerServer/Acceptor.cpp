#include "Acceptor.h"
#include "Epoll.h"
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "Socket.h"

const int LISTENQ = 100;

Acceptor::Acceptor(int port, int threadsNum)
:_listenfd(socket_bind(port))
,_epoll(new Epoll())
,_eventLoopThreadPool(new EventLoopThreadPool(threadsNum))
{
    listen(_listenfd, LISTENQ);
    _epoll->add_event(_listenfd, EPOLLIN);
    _eventLoopThreadPool->start();
}

Acceptor::~Acceptor()
{
    close(_listenfd);
}


void Acceptor::loop()
{
    while(1)
    {
        _epoll->wait_event();
        if(_epoll->_eventNum)
        {
            int clientfd;
            struct sockaddr_in clientaddr;
            socklen_t clientaddr_len = sizeof(clientaddr);
            clientfd = accept(_listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
            if(clientfd == -1)
            {
                perror("accept error");
                exit(1);
            }
            printf("new connection from %s:%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            shared_ptr<EventLoopThread> sp = _eventLoopThreadPool->getNextEventLoopThread();
            sp->getEventLoop()->addToEpoll(clientfd);
        }
    }

}