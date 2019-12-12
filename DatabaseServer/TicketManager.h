#ifndef TICKETMANAGER_H
#define TICKETMANAGER_H

#include <string>
#include <mysql/mysql.h>
#include <assert.h>
#include <map>
#include <vector>
#include "MutexLock.h"
using std::string;
using std::map;
using std::vector;

struct Ticket
{
    Ticket():_valid(true),_mutex(){}
    Ticket(string s,string e, string x):_valid(true),_mutex(),_id(x)
    {
        int start = atoi(s.c_str());
        int end = atoi(e.c_str());
        assert(start < end && start>=1 && end<=33);
        _bits = 0;
        for(int i = start; i < end; i++)
            _bits |= (1<<(i-1));
    }

    bool operator < (const Ticket &t) const
    {
        if(_bits != t._bits)
            return __builtin_popcount(_bits) < __builtin_popcount(t._bits);
        return _id < t._id;
    }

    unsigned _bits;
    string _id;
    bool _valid;
    MutexLock _mutex;
};

class TicketManager
{
    public:
        TicketManager();
        ~TicketManager();
        void update();
        void loop();
    private:
        MYSQL *conn;
        MYSQL_FIELD *field;
        MYSQL_RES *res_ptr;
        MYSQL_ROW result_row;
};

#endif