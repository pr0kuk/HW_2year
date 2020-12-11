#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
    int id_key = shmget(42, sizeof(unsigned int), IPC_EXCL);
    pid_t* pid = (pid_t *)shmat(id_key, NULL, 0);
    if (argv[1][0] == 'k')
    {
        kill(*pid, SIGINT);
        printf("%d daemon terminated\n", *pid);
    }
    if (argv[1][0] == 'b')
    {
        id_key = shmget(24, 4, IPC_CREAT | 0666);
        char* new_dir = (char*)shmat(id_key, NULL, 0);
        strcpy(new_dir, argv[2]);
        shmdt(new_dir);
        kill(*pid, SIGUSR1);
        printf("backup directory changed to %s\n", argv[2]);
    }
    if (argv[1][0] == 'w')
    {
        id_key = shmget(28, 20, IPC_CREAT | 0666);
        char* new_dir = (char*)shmat(id_key, NULL, 0);
        strcpy(new_dir, argv[2]);
        shmdt(new_dir);
        kill(*pid, SIGUSR2);
        printf("working directory changed to %s\n", argv[2]);
    }
    if (argv[1][0] == 'p')
        printf("pid is %d\n", *pid);
    return 0;
}