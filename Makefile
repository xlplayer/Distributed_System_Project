OBJDIR = obj
PRODUCEROBJ = ${OBJDIR}/producer
CONSUMEROBJ = ${OBJDIR}/consumer
DATABASEOBJ = ${OBJDIR}/database
MQOBJ = ${OBJDIR}/mq

PRODUCERDIR = ProducerServer
CONSUMERDIR = ConsumerServer
DATABASEDIR = DatabaseServer
MQDIR = MQServer

PRODUCERSOURCES = $(PRODUCERDIR)/*.cpp
CONSUMERSOURCES = $(CONSUMERDIR)/*.cpp
DATABASESOURCES = $(DATABASEDIR)/*.cpp
MQSOURCES = $(MQDIR)/*.cpp


CC = g++
CFLAGS = -std=c++11 -O3 -lpthread

all: producer consumer database mq

producer : $(PRODUCERSOURCES)
	$(CC) -o $(PRODUCEROBJ) $(PRODUCERSOURCES)  $(CFLAGS) -lhiredis

consumer : $(CONSUMERSOURCES)
	$(CC) -o $(CONSUMEROBJ) $(CONSUMERSOURCES)  $(CFLAGS)

database : $(DATABASESOURCES)
	$(CC) -o $(DATABASEOBJ) $(DATABASESOURCES)  $(CFLAGS) -lmysqlclient

mq : $(MQSOURCES)
	$(CC) -o $(MQOBJ) $(MQSOURCES) $(CFLAGS)

clean :
	rm -f $(OBJDIR)