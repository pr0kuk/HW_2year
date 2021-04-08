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

#define PATH "/tmp/mysock"
#define BUFSZ 256
#define IDSZ 16
#define MAX_CLIENTS 10000
#define CLOCKS_TO_WAIT 1000
#define POLL_WAIT 100
#define KEY 9973
#define PORT 23456

#endif