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
    if (argv[1][0] == 'd')
    {
        kill(*pid, SIGUSR1);
        id_key = shmget(24, 1, IPC_EXCL);
        char* new_dir = (char*)shmat(id_key, NULL, 0);
        printf("backup directory changed to %s\n", argv[2]);
        execl("/home/alexshch/HW_2year/daemon/a.out", " ", new_dir, argv[2], NULL);
        perror("execlp");
    }
    if (argv[1][0] == 'p')
        printf("pid is %d\n", *pid);
    return 0;
}