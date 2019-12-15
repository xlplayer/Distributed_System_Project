#include "Acceptor.h"
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


int main(int argc, char **argv)
{
    int port = 50000;
    int threadsNum = 4;
    int opt;
    const char *str = "t:p:";
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
    shared_ptr<Acceptor> acceptor(new Acceptor(port, threadsNum));
    acceptor->loop();
}