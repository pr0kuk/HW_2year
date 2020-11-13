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
    int id = msgget(key, IPC_CREAT | 0666);
    pid_t pid = getpid();
    struct msgbuf msg = {1, pid};
    msgctl(id, IPC_RMID, 0);
    id = msgget(key, IPC_CREAT | 0666);
    msgsnd(id, &msg, sizeof(struct msgbuf), 0);
    printf("sent: %d\n", msg.pid);
    msgrcv(id, &msg, sizeof(struct msgbuf), 2, 0);
    printf("received: %d\n", msg.pid);
    msgctl(id, IPC_RMID, 0);
    return 0;
}