OBJDIR = obj
SRCDIR = src

PRODUCERSOURCES = ${SRCDIR}/ProducerWorker/*.cpp ${SRCDIR}/base/*.cpp
CONSUMERSOURCES = ${SRCDIR}/ConsumerWorker/*.cpp ${SRCDIR}/base/*.cpp
DATABASESOURCES = ${SRCDIR}/DatabaseServer/*.cpp ${SRCDIR}/base/*.cpp
MQSOURCES = ${SRCDIR}/MQServer/*.cpp ${SRCDIR}/base/*.cpp


CC = g++

debug = false
ifeq ($(debug), false)
CXXFLAGS = -I ./$(SRCDIR) -std=c++11 -O3 -lpthread
else
CXXFLAGS = -I ./$(SRCDIR) -std=c++11 -g -D DEBUG -lpthread
endif

all: producer consumer database mq

producer : $(PRODUCERSOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS) -lhiredis

consumer : $(CONSUMERSOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS)

database : $(DATABASESOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS) -lmysqlclient

mq : $(MQSOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS)

clean :
	rm -f $(OBJDIR)