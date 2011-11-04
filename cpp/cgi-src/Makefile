
all: cgi

#from command: ccgnu2-config --flags --stdlibs
CCPPI=-I/usr/include/cc++2 -D_GNU_SOURCE
CCPP=-I/usr/include/cc++2 -D_GNU_SOURCE -L/usr/lib -lccext2 -lccgnu2 -L/usr/lib -lxml2 -lz -lm -lz -ldl -lrt -pthread

SOCK_O = ../socket_tools.o ../thread_tools.o ../definitions.o

#modules
cgi_tools.o: cgi_tools.C cgi_tools.h

#cgi clients
cgi: index.cgi play.cgi read.cgi help.cgi
index.cgi: index.C cgi_tools.h cgi_tools.o
	g++ index.C cgi_tools.o -ggdb -lcgicc -o index.cgi
play.cgi: play.C cgi_tools.h cgi_tools.o
	g++ play.C cgi_tools.o $(SOCK_O) -ggdb -lcgicc -lpthread -lbz2 -o play.cgi
ask.cgi: ask.C cgi_tools.h cgi_tools.o
	g++ ask.C cgi_tools.o -ggdb -lcgicc -o ask.cgi
teach.cgi: teach.C cgi_tools.h cgi_tools.o
	g++ teach.C cgi_tools.o -ggdb -lcgicc -o teach.cgi
read.cgi: read.C cgi_tools.h cgi_tools.o
	g++ read.C cgi_tools.o -ggdb -lcgicc -o read.cgi
help.cgi: help.C cgi_tools.h cgi_tools.o
	g++ help.C cgi_tools.o -ggdb -lcgicc -o help.cgi

SOCKET=/tmp/test_socket
test: echo_server
	./echo_server

clean:
	rm -f core core.* *.o *.cgi *.html



