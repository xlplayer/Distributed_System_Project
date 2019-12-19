#include "base/Thread.h"
#include "EventLoopThreadPool.h"
#include "base/rapidjson/document.h"
#include "base/rapidjson/writer.h"
#include "base/rapidjson/stringbuffer.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <fstream>
using namespace rapidjson;
using std::shared_ptr;
using std::vector;
using std::string;
using std::ifstream;

vector<sockaddr_in> queueServerAddr,databaseServerAddr;
void initServerAddr(string &path, vector<sockaddr_in> &queueServerAddr, vector<sockaddr_in> &databaseServerAddr);

int main(int argc, char **argv)
{
    char buf[1024];
    if(!getcwd(buf,1024)) perror("getcwd error!\n");
    string path = string(buf) + "/config.json";

    int threadsNum = 4;
    int opt;
    const char *str = "t:c:";
    while((opt = getopt(argc, argv, str)) != -1)
    {
        switch(opt)
        {
            case 't':
            {
                threadsNum = atoi(optarg);
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
    initServerAddr(path, queueServerAddr, databaseServerAddr);
    shared_ptr<EventLoopThreadPool> _etp (new EventLoopThreadPool(threadsNum));
    _etp->start();
    printf("consume worker 启动成功!\n");
    //while(1);
}

void initServerAddr(string &path, vector<sockaddr_in> &queueServerAddr, vector<sockaddr_in> &databaseServerAddr)
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

    Value &database = d["database"];
    for(size_t i=0;i<database.Size();i++)
    {
        Value &v = database[i];
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(v["port"].GetUint());
        inet_pton(AF_INET, v["ip"].GetString(), &servaddr.sin_addr);
        databaseServerAddr.push_back(servaddr);
    }
}