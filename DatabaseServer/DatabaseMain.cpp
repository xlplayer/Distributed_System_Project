#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "Epoll.h"
#include "Acceptor.h"
#include "Thread.h"
#include "TicketManager.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <queue>
using std::shared_ptr;
using std::queue;
using std::make_shared;

map<string, vector<Ticket> > tickets;

int main(int argc, char **argv)
{
    int port = 80;
    int threadsNum = 4;
    int opt;
    const char *str = "t:l:p:";
    while((opt = getopt(argc, argv, str)) != -1)
    {
        switch(opt)
        {
            case 't':
            {
                threadsNum = atoi(optarg);
                break;
            }
            case 'p':
            {
                port = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
    shared_ptr<TicketManager> ticketManager(new TicketManager());
    shared_ptr<Thread> ticketThread(new Thread(bind(&TicketManager::loop,ticketManager)));
    ticketThread->start();

    shared_ptr<Acceptor> acceptor(new Acceptor(port, threadsNum));
    acceptor->loop();
}