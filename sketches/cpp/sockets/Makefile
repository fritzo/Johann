
all: stream_client stream_server dgram_client dgram_server cc_server

definitions.o: definitions.C definitions.h
	g++ -c definitions.C

stream_client: stream_client.C
	g++ stream_client.C -o stream_client
stream_server: stream_server.C
	g++ stream_server.C -o stream_server

sockets.o: sockets.C sockets.h definitions.h
	g++ -c sockets.C
dgram_client: dgram_client.C sockets.h definitions.h sockets.o definitions.o
	g++ dgram_client.C sockets.o definitions.o -o dgram_client
dgram_server: dgram_server.C sockets.h definitions.h sockets.o definitions.o
	g++ dgram_server.C sockets.o definitions.o -o dgram_server

#from command: ccgnu2-config --flags --stdlibs
CCPP=-I/usr/include/cc++2 -D_GNU_SOURCE -L/usr/lib -lccext2 -lccgnu2 -L/usr/lib -lxml2 -lz -lm -lz -ldl -lrt -pthread

cc_server: cc_server.C socket_tools.h socket_tools.o
	g++ cc_server.C socket_tools.o $(CCPP) -o cc_server

sclean:
	rm -f /tmp/johann_*_socket*
clean:
	rm -f *_client *_server *.o

