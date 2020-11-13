#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/msg.h>

struct msgbuf 
{
    long int dir;
    pid_t pid;
};

int main()
{
    char* path = "rec.c";
    key_t key = ftok(path, 1);    
    int id = msgget(key, 0);
    pid_t pid = getpid();
    struct msgbuf msg;
    msgrcv(id, &msg, sizeof(struct msgbuf), 1, 0);
    printf("received: %d\n", msg.pid);
    msg.pid = pid;
    msg.dir = 2;
    msgsnd(id, &msg, sizeof(struct msgbuf), 0);
    printf("sent: %d\n", pid);
    return 0;
}