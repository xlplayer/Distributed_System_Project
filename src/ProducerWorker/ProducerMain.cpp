#include "Acceptor.h"
#include "base/Thread.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <fstream>
#include "MQhandler.h"
#include "MessageQueue.h"
#include "base/rapidjson/document.h"
#include "base/rapidjson/stringbuffer.h"
#include "base/rapidjson/writer.h"
using namespace rapidjson;
using std::shared_ptr;
using std::queue;
using std::ifstream;


MessageQueue msgQueue;
vector<sockaddr_in> queueServerAddr;
void initQueueServerAddr(string &path, vector<sockaddr_in> &queueServerAddr);

int main(int argc, char **argv)
{
    char buf[1024];
    if(!getcwd(buf,1024)) perror("getcwd error!\n");
    string path = string(buf) + "/config.json";

    int port = 80;
    int threadsNum = 4;
    int opt;
    const char *str = "t:p:c:";
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
            case 'c':
            {
                path = optarg;
                break;
            }
            default:
                break;
        }
    }

    initQueueServerAddr(path, queueServerAddr);

    shared_ptr<MQhandler> msgProducer(new MQhandler());
    shared_ptr<Thread> msgThread(new Thread(bind(&MQhandler::loop,msgProducer)));
    msgThread->start();

    shared_ptr<Acceptor> acceptor(new Acceptor(port, threadsNum));
    acceptor->loop();
}

void initQueueServerAddr(string &path, vector<sockaddr_in> &queueServerAddr)
{
    ifstream fin(path);
    if(!fin.is_open())
    {
        printf("当前路径:%s配置文件不存在, 请使用-c指定正确的配置文件路径\n",path.c_str());
        exit(1);
    }
    string str,line;
    while(getline(fin, line)) str += line;
    Document d;
    d.Parse(str.c_str());

    Value &mq = d["mq"];
    for(size_t i=0;i<mq.Size();i++)
    {
        Value &v = mq[i];
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(v["port"].GetUint());
        inet_pton(AF_INET, v["ip"].GetString(), &servaddr.sin_addr);
        queueServerAddr.push_back(servaddr);
    }
}