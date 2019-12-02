#include "Epoll.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <queue>
#include <mysql/mysql.h>
#include <iostream>
using namespace std;
using namespace rapidjson;
using std::shared_ptr;
using std::queue;
using std::string;

const int LISTENQ = 5;
const int BUFFSIZE = 1024;
char buf[BUFFSIZE];
queue<string> MQ;
const int MAXQUEUESIZE = 102400;

int socket_bind(int port);
void handle_accept(shared_ptr<Epoll> _epoll, int listenfd);
void handle_read(shared_ptr<Epoll> _epoll, int fd);

void getTrains(string &start, string &end, vector<string> &trains);
void getLeftNums(string &train, string &date, string &start, string &end, string &result);
void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result);

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
    int listenfd = socket_bind(port);
    listen(listenfd, LISTENQ);
    shared_ptr<Epoll> _epoll(new Epoll());
    _epoll->add_event(listenfd, EPOLLIN);

    while(1)
    {
        _epoll->wait_event();
        for(int i=0; i<_epoll->_eventNum; i++)
        {
            int fd = _epoll->_events[i].data.fd;
            if(fd == listenfd &&_epoll->_events[i].events & EPOLLIN)
                handle_accept(_epoll, listenfd);
            else if(fd != listenfd &&_epoll->_events[i].events & EPOLLIN)
                handle_read(_epoll, fd);
        }
    }
}

void handle_read(shared_ptr<Epoll> _epoll, int fd)
{
    int nread = read(fd, buf, BUFFSIZE);
    if(nread == -1)
    {
        perror("read error");
        close(fd);
        _epoll->del_event(fd, EPOLLIN);
    }
    else if(nread == 0)
    {
        fprintf(stderr,"client close.\n");
        close(fd);
        _epoll->del_event(fd, EPOLLIN);
    }
    else
    {
        printf("read message: ");
        for(int i=0;i<nread;i++) putchar(buf[i]);
        printf("\n");
        
        Document d;
        d.Parse(buf, nread);
        string operate = d["operate"].GetString();
        if(operate == "query_train")
        {
            string start,end;
            start = d["start"].GetString();
            end = d["end"].GetString();
            vector<string> trains;
            getTrains(start, end, trains);

            StringBuffer s;
            Writer<StringBuffer> writer(s);
            writer.StartObject();
            writer.Key("result");
            writer.StartArray();
            for(size_t i = 0; i<trains.size();i++)
                writer.String(trains[i].c_str());
            writer.EndArray();
            writer.EndObject();
            write(fd, s.GetString(), s.GetSize());

        }
        else if(operate == "query_leftnum")
        {
            string train, date, start, end;
            train = d["train"].GetString();
            date = d["date"].GetString();
            start = d["start"].GetString();
            end = d["end"].GetString();
            string result;
            getLeftNums(train, date, start, end, result);

            StringBuffer s;
            Writer<StringBuffer> writer(s);
            writer.StartObject();
            writer.Key("result");
            writer.String(result.c_str());
            writer.EndObject();
            write(fd, s.GetString(), s.GetSize());
        }
        else if(operate == "buy")
        {
            string date,train_number,start,end,passenger_name,passenger_id;
            date = d["date"].GetString();
            train_number = d["train_number"].GetString();
            start = d["start"].GetString();
            end = d["end"].GetString();
            passenger_name = d["passenger_name"].GetString();
            passenger_id = d["passenger_id"].GetString();
            string result;
            buyTicket(date, train_number, start, end, passenger_name, passenger_id, result);
            write(fd, result.c_str(), result.length());
        }
        else if(operate == "pay")
        {

        }
    }
}

void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result)
{
    string start_idx, end_idx;
    bool success = true;
    string id, seat_number, compartment_number;
    string head, tail;
    char query[1024];
    MYSQL *conn;
    MYSQL_FIELD *field;
    MYSQL_RES *res_ptr;
    MYSQL_ROW result_row;

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, CLIENT_FOUND_ROWS);
    mysql_query(conn, "set names utf8");

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train_number.c_str(), start.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    start_idx = mysql_fetch_row(res_ptr)[0];

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train_number.c_str(), end.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    end_idx = mysql_fetch_row(res_ptr)[0];

    mysql_autocommit(conn, false);
    do{
        sprintf(query, "select * from %s_%s where start <= %s and end >= %s and status = \"unsold\" limit 1 for update", date.c_str(),train_number.c_str(),start_idx.c_str(), end_idx.c_str());
        cout<<query<<endl;
        if(mysql_query(conn, query))
        { 
            success = false;
            break;
        } 
        res_ptr = mysql_store_result(conn);
        result_row = mysql_fetch_row(res_ptr);
        id = result_row[0];
        head = result_row[2];
        tail = result_row[3];
        seat_number = result_row[4];
        compartment_number = result_row[5];

        sprintf(query, "update %s_%s set status = \"unpaid\", start = %s, end = %s, passenger_name = \"%s\", passenger_id = \"%s\" \
                        where id = %s", date.c_str(),train_number.c_str(), start_idx.c_str(), end_idx.c_str(), passenger_name.c_str(), passenger_id.c_str(), id.c_str());
        cout<<query<<endl;
        if (mysql_query(conn, query)) 
        { 
            success = false;
            break;
        }

        if(atoi(start_idx.c_str()) > atoi(head.c_str()))
        {
            sprintf(query, "insert into %s_%s (status,start,end,seat_number,compartment_number) values (\"unsold\",%s,%s,%s,%s)", 
                            date.c_str(),train_number.c_str(),head.c_str(), start_idx.c_str(), seat_number.c_str(), compartment_number.c_str());
            cout<<query<<endl;
            if (mysql_query(conn, query)) 
            { 
                success = false;
                break;
            }  
        }

        if(atoi(end_idx.c_str()) < atoi(tail.c_str()))
        {
            sprintf(query, "insert into %s_%s (status,start,end,seat_number,compartment_number) values (\"unsold\",%s,%s,%s,%s)", 
                            date.c_str(),train_number.c_str(),end_idx.c_str(), tail.c_str(), seat_number.c_str(), compartment_number.c_str());
            cout<<query<<endl;
            if (mysql_query(conn, query)) 
            { 
                success = false;
                break;
            }  
        }
        
    }while(false);

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    if(success == false)
    {
        mysql_rollback(conn);
        mysql_autocommit(conn, true);
        writer.Key("result");
        writer.String("failure");
    }
    else
    {
        mysql_commit(conn);
        mysql_autocommit(conn, true);
        writer.Key("result");
        writer.String("success");
        writer.Key("train_number");
        writer.String(train_number.c_str());
        writer.Key("start");
        writer.String(start.c_str());
        writer.Key("end");
        writer.String(end.c_str());
        writer.Key("compartment_number");
        writer.String(compartment_number.c_str());
        writer.Key("seat_number");
        writer.String(seat_number.c_str());
    }
    writer.EndObject();
    result = s.GetString();
    
    mysql_free_result(res_ptr);
    mysql_close(conn);
    mysql_library_end();
}

void getTrains(string &start, string &end, vector<string> &trains)
{
    char query[1024];
    MYSQL *conn;
    MYSQL_FIELD *field;

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, CLIENT_FOUND_ROWS);
    mysql_query(conn, "set names utf8");
    sprintf(query, "select * from station_train where start = \"%s\" and end = \"%s\"", start.c_str(), end.c_str());
    cout<<query<<endl;
    int res = mysql_query(conn, query);
    if (res) 
	{ 
        printf("Error： mysql_query !\n");
        mysql_close(conn);
        mysql_library_end();
    }
    else 
    { 
        MYSQL_RES *res_ptr = mysql_store_result(conn);
        if (res_ptr) 
        {
            int column = mysql_num_fields(res_ptr);
            int row = mysql_num_rows(res_ptr);
            for (int i = 0; field = mysql_fetch_field(res_ptr); i++)
                printf("%s\t", field->name);
            printf("\n");
            for (int i = 0; i < row; i++)
            {
                MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
                for (int j = 0; j < column; j++)
                    printf("%s\t", result_row[j]);
                printf("\n");
                trains.push_back(result_row[3]);
            }

        }
        mysql_free_result(res_ptr);
        mysql_close(conn);
        mysql_library_end();
    }
}

void getLeftNums(string &train, string &date, string &start, string &end, string &result)
{
    string start_idx, end_idx;
    char query[1024];
    int res;
    MYSQL *conn;
    MYSQL_FIELD *field;
    MYSQL_RES *res_ptr;

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, CLIENT_FOUND_ROWS);
    mysql_query(conn, "set names utf8");

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train.c_str(), start.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    start_idx = mysql_fetch_row(res_ptr)[0];

    sprintf(query, "select idx from station_idx where train = \"%s\" and station = \"%s\"", train.c_str(), end.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    end_idx = mysql_fetch_row(res_ptr)[0];

    sprintf(query, "select count(*) from %s_%s where start <= %s and end >= %s and status = \"unsold\"", date.c_str(), train.c_str(), start_idx.c_str(), end_idx.c_str());
    cout<<query<<endl;
    mysql_query(conn, query);
    res_ptr = mysql_store_result(conn);
    string left_num = mysql_fetch_row(res_ptr)[0];

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("train_number");
    writer.String(train.c_str());
    writer.Key("start");
    writer.String(start.c_str());
    writer.Key("end");
    writer.String(end.c_str());
    writer.Key("num");
    writer.Uint(atoi(left_num.c_str()));
    writer.EndObject();
    result = s.GetString();

    mysql_free_result(res_ptr);
    mysql_close(conn);
    mysql_library_end();
}

void handle_accept(shared_ptr<Epoll> _epoll,int listenfd)
{
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
    if(clientfd == -1)
    {
        perror("accept error");
        exit(1);
    }
    printf("new connection from %s:%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    _epoll->add_event(clientfd, EPOLLIN);
}

int socket_bind(int port)
{
    if(port < 0|| port > 65535) return -1;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind error");
        exit(1);
    }
    return listenfd;
}
