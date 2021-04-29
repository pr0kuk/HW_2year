OBJECTS = $(SOURCES:.c=.o)

.PHONY: all $(OBJECTS)
all: dfke.o server.o client.o con.o log.o tcp_client.o tcp.o udp.o
	gcc -shared dfke.o -o libdfke.so -lm
	gcc client.o -o client -ldfke
	gcc con.o -o con
	gcc -shared tcp.o log.c -o libtcpl.so -ldfke
	gcc -shared udp.o log.c -o libudp.so -ldfke
	gcc tcp_client.o -o tcp_client -ldfke
	gcc server.o log.c -o server -ldl

dfke.o: dfke.c
	gcc dfke.c -c -o dfke.o -g -O0 -lm -fPIC

tcp.o: tcp.c
	gcc -c tcp.c -o tcp.o -g -O0 -ldfke -fPIC

udp.o: udp.c
	gcc -c udp.c -o udp.o -g -O0 -ldfke -fPIC

server.o: server.c
	gcc server.c -c -o server.o -g -O0 -ldl -ldfke -fPIC

client.o: client.c
	gcc client.c -c -o client.o -ldfke -fPIC
	
log.o: log.c
	gcc log.c -c -o log.o
	
con.o: con.c
	gcc con.c -c -o con.o
	
tcp_client.o: tcp_client.c
	gcc tcp_client.c -c -o tcp_client.o -ldfke



.PHONY: clean
clean:
	-rm *.o
	-rm server
	-rm client
