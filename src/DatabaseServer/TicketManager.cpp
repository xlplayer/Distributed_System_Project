#include "TicketManager.h"
#include "assert.h"
#include <vector>
#include <sys/time.h>
#include <unistd.h>
using std::vector;

extern  map<string, vector<Ticket> > tickets;

TicketManager::TicketManager()
{
    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "1234", "train", 0, NULL, 0);
    mysql_query(conn, "set names utf8");

    update();
}

TicketManager::~TicketManager()
{}

void TicketManager::update()
{
    char query[1024];
    vector<string> trains;
    mysql_query(conn, "select * from trains");
    res_ptr = mysql_store_result(conn);
    int row = mysql_num_rows(res_ptr);
    for (int i = 0; i < row; i++)
    {
        result_row = mysql_fetch_row(res_ptr);
        trains.push_back(result_row[1]);
    }
    for(int i = 0; i < row; i++) tickets[trains[i]].reserve(1000000);//初始化vector大小，防止vector自动扩容

    for(int i=0;i<trains.size();i++)
    {
        sprintf(query, "select * from %s where status = \"unsold\"", trains[i].c_str());
        mysql_query(conn, query);
        res_ptr = mysql_store_result(conn);
        int row = mysql_num_rows(res_ptr);
        for (int r = 0; r < row; r++)
        {
            result_row = mysql_fetch_row(res_ptr);
            tickets[trains[i]].push_back(Ticket(result_row[2],result_row[3],result_row[0]));
        }
    }
    mysql_free_result(res_ptr);
}

void TicketManager::loop()
{
    int day = -1;
    while(1)
    {
        struct timeval cur;
        gettimeofday(&cur,NULL);
        time_t time = cur.tv_sec;
        struct tm* p_time = localtime(&time);
        #ifdef DEBUG
        printf("day:%d",p_time->tm_mday);
        #endif
        if(p_time->tm_hour == 4 && p_time->tm_mday != day)
        {
            day = p_time->tm_mday;
            tickets.clear();
            update();
        }
        sleep(1200);
    }
}