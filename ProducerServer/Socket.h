#ifndef SOCKET_H
#define SOCKET_H

#include <string>
using std::string;

int socket_bind(int port);

int readn(int fd, string &msg, bool &zero);

int writen(int fd, string &msg);

int writen(int fd, void* buff, int size);//for eventfd

void setSocketNonBlocking(int fd);

#endif