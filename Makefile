OBJDIR = obj
SRCDIR = src

PRODUCERSOURCES = ${SRCDIR}/ProduceWorker/*.cpp ${SRCDIR}/base/*.cpp
CONSUMERSOURCES = ${SRCDIR}/ConsumeWorker/*.cpp ${SRCDIR}/base/*.cpp
DATABASESOURCES = ${SRCDIR}/DatabaseServer/*.cpp ${SRCDIR}/base/*.cpp
MQSOURCES = ${SRCDIR}/MQServer/*.cpp ${SRCDIR}/base/*.cpp


CC = g++

debug = false
ifeq ($(debug), false)
CXXFLAGS = -I ./$(SRCDIR) -std=c++11 -g -lpthread
else
CXXFLAGS = -I ./$(SRCDIR) -std=c++11 -g -D DEBUG -lpthread
endif

all: produceworker consumeworker databaseserver mqserver

produceworker : $(PRODUCERSOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS) -lhiredis

consumeworker : $(CONSUMERSOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS)

databaseserver : $(DATABASESOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS) -lmysqlclient

mqserver : $(MQSOURCES)
	$(CC) -o $(OBJDIR)/$@ $^  $(CXXFLAGS)

.PHONY : clean
clean :
	rm -f $(OBJDIR)/*