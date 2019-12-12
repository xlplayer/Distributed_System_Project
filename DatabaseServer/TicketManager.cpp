#include "TicketManager.h"
#include "assert.h"
#include <vector>
#include <iostream>
using namespace std;
using std::vector;

extern   map<string, vector<Ticket> > tickets;

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
    // for(auto iter : _tickets)
    // {
    //     cout<<iter.first<<endl;
    //     for(auto t : iter.second)
    //     {
    //         cout<<t._bits<<" "<<t._id<<endl;
    //     }
    // }
    mysql_free_result(res_ptr);
}

void TicketManager::loop()
{
    while(1)
    {

    }
}