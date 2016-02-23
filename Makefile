OBJS = lib/InvertedIndexGen.o lib/FileIndex.o lib/Util.o \
       lib/MemPool.o lib/URLParser.o lib/csapp.o lib/HTTPLib.o lib/Spider.o
IDXR = lib/Main.o
URLT = lib/urltest.o
HTTP = lib/httptest.o
SPID = lib/spidey.o
TINY = lib/tiny.o

CPP = g++
CC  = gcc
DEBUG = -g
CFLAGS = -Wall -c -g
LFLAGS = -pthread -Wall -g

IDXRP = InvIndexer
URLTP = urltest
HTTPP = httptest
SPIDP = spidey
TINYP = tiny
PROGS = $(IDXRP) $(URLTP) $(HTTPP) $(SPIDP) $(TINYP)

# Set this to nothing if you are running on a platform different than
# Linux.  Linux requires this library to be linked to use the clock
# functions used in Spider.cc
RT = 

all: init idxr spidey tiny

test: all urlt http

idxr: $(OBJS) $(IDXR)
	$(CPP) $(LFLAGS) $(OBJS) $(IDXR) -o $(IDXRP) $(RT)

#cheng: 
#	g++ -g myInvertedIndex.cpp -o myInvertedIndex

urlt: $(OBJS) $(URLT)
	$(CPP) $(LFLAGS) $(OBJS) $(URLT) -o $(URLTP) $(RT)

http: $(OBJS) $(HTTP)
	$(CPP) $(LFLAGS) $(OBJS) $(HTTP) -o $(HTTPP) $(RT)

spidey: $(OBJS) $(SPID)
	$(CPP) $(LFLAGS) $(OBJS) $(SPID) -o $(SPIDP) $(RT)

tiny: $(OBJS) $(TINY)
	$(CPP) $(LFLAGS) -g $(OBJS) $(TINY) -o $(TINYP) $(RT)

run: all
	./InvIndexer files.txt

init: 
	if [ ! -d lib ]; then mkdir lib; fi

lib/Main.o: src/Main.cpp src/InvertedIndexGen.h
	$(CPP) $(CFLAGS) src/Main.cpp -o $@

lib/FileIndex.o: src/FileIndex.cpp src/FileIndex.h
	$(CPP) $(CFLAGS) src/FileIndex.cpp -o $@

lib/InvertedIndexGen.o: src/InvertedIndexGen.cpp src/InvertedIndexGen.h src/FileIndex.h
	$(CPP) $(CFLAGS) src/InvertedIndexGen.cpp -o $@

lib/Util.o: src/Util.cpp src/Util.h
	$(CPP) $(CFLAGS) src/Util.cpp -o $@

lib/MemPool.o: src/MemPool.cpp
	$(CPP) $(CFLAGS) src/MemPool.cpp -o $@

lib/URLParser.o: src/URLParser.cc src/URLParser.h
	$(CPP) $(CFLAGS) src/URLParser.cc -o $@

lib/HTTPLib.o: src/HTTPLib.cc src/HTTPLib.h
	$(CPP) $(CFLAGS) src/HTTPLib.cc -o $@

lib/Spider.o: src/Spider.cc src/Spider.h
	$(CPP) $(CFLAGS) src/Spider.cc -o $@

lib/csapp.o: src/csapp.c src/csapp.h
	$(CC) $(CFLAGS) src/csapp.c -o $@

lib/urltest.o: src/urltest.cc src/URLParser.h
	$(CPP) $(CFLAGS) src/urltest.cc -o $@

lib/httptest.o: src/httptest.cc
	$(CPP) $(CFLAGS) src/httptest.cc -o $@

lib/spidey.o: src/spidey.cc
	$(CPP) $(CFLAGS) src/spidey.cc -o $@

lib/tiny.o: src/tiny.c
	$(CC) $(CFLAGS) src/tiny.c -o $@

clean:
	rm -rf lib
	find ./ -name "*~" | xargs rm -f
	rm -f $(PROGS)
