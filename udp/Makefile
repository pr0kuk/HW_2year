OBJECTS = $(SOURCES:.c=.o)

.PHONY: all $(OBJECTS)
all: server.o client.o con.o log.o tcp_client.o tcp.o udp.o
	gcc client.o -o client
	gcc con.o -o con
	gcc -shared tcp.o -o libtcpl.so
	gcc -shared udp.o -o libudp.so
	gcc tcp_client.o -o tcp_client
	gcc server.o log.c -o server -L $(shell pwd) -ludp -ldl -fPIC

tcp.o: tcp.c
	gcc -c tcp.c -o tcp.o -fPIC

udp.o: udp.c
	gcc -c udp.c -o udp.o -fPIC

server.o: server.c
	gcc server.c -c -o server.o -L $(shell pwd) -ludp -ldl

client.o: client.c
	gcc client.c -c -o client.o
	
log.o: log.c
	gcc log.c -c -o log.o
	
con.o: con.c
	gcc con.c -c -o con.o
	
tcp_client.o: tcp_client.c
	gcc tcp_client.c -c -o tcp_client.o
	
	
.PHONY: clean
clean:
	-rm *.o
	-rm server
	-rm client
