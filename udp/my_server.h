#ifndef _myserver_h
#define _myserver_h
#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <limits.h>
#include <sys/un.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <threads.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <dlfcn.h>

#define PATH "/tmp/mysock"
#define BUFSZ 256
#define IDSZ 10
#define MAX_CLIENTS 10000
#define CLOCKS_TO_WAIT 1000
#define POLL_WAIT 100
#define KEY 9973
#define PORT 23456
#define SHMKEY 42
#define CRPTKEY 0x7F
#define crypto(buffer) do {for (int i = 0; i < BUFSZ; (buffer[i++]) ^= K);} while(0);

struct funcs {
    int (*send_info)(int, char*, struct sockaddr*);
    int (*settings)(int *, int*, struct sockaddr_in*);
    int (*server_handler)(int *, int*, int(*)[2], struct sockaddr_in*, int*, int (*execution)(char*, int, struct sockaddr*, int*), void (*off_bash)(int));
    int (*executionf)(char*, int, struct sockaddr*, int*);
    void (*off_bashf)(int);
};

char gen_open_key_server(long double g, long double a, long double p);
char gen_open_key_client(long double g, long double b, long double p);
char gen_close_key_server(char B, long double a, long double p);
char gen_close_key_client(char A, long double b, long double p);
    

#endif