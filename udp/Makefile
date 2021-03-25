OBJECTS = $(SOURCES:.c=.o)

.PHONY: all $(OBJECTS)
all: server.o client.o
	gcc server.o log.c -o server
	gcc client.o -o client

server.o: server.c
	gcc server.c -c -o server.o

client.o: client.c
	gcc client.c -c -o client.o
	
log.o: log.c
	gcc log.c -c -o log.o

.PHONY: clean
clean:
	-rm *.o
	-rm server
	-rm client
