#ifndef DARABASE_H
#define DATABASE_H

#include <mysql/mysql.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <assert.h>

using std::vector;
using std::string;
using std::map;
using std::set;

struct Ticket
{
    Ticket(){}
    Ticket(string s,string e, string x)
    {
        id = x;

        int start = atoi(s.c_str());
        int end = atoi(e.c_str());
        assert(start < end && start>=1 && end<=33);
        bit_status = 0;
        for(int i = start; i < end; i++)
            bit_status |= (1<<(i-1));
    }
    unsigned bit_status;
    string id;
    bool operator < (const Ticket &t) const
    {
        if(bit_status != t.bit_status)
            return __builtin_popcount(bit_status) < __builtin_popcount(t.bit_status);
        return id < t.id;
    }
};

class Database
{
    public:
        Database();
        ~Database();
        void getTrains(string &start, string &end, vector<string> &trains);
        void getLeftNums(string &train, string &date, string &start, string &end, string &result);
        void buyTicket(string &date, string &train_number, string &start, string &end, string &passenger_name, string &passenger_id, string &result);

    private:
        map<string, set<Ticket> > _tickets;
        MYSQL *conn;
        MYSQL_FIELD *field;
        MYSQL_RES *res_ptr;
        MYSQL_ROW result_row;
};
#endif