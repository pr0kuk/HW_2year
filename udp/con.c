#include "my_server.h"
int main()
{
    int id_key = shmget(SHMKEY, sizeof(unsigned int), IPC_EXCL);
    pid_t* pid = (pid_t *)shmat(id_key, NULL, 0);
    printf("gid is %d\n", *pid);
    if (kill(*pid, SIGINT) < 0) {
        perror("kill");
        return -1;
    }
    printf("SIGINT sent to %d\n", *pid);
    return 0;
}