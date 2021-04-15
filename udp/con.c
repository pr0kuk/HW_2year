#include "my_server.h"
int main()
{
    int id_key = shmget(SHMKEY, sizeof(unsigned int), IPC_EXCL);
    pid_t* pid = (pid_t *)shmat(id_key, NULL, 0);
    kill(*pid, SIGINT);
    printf("SIGINT sent to %d\n", *pid);
    return 0;
}