#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <assert.h>
#include <sys/shm.h>
#include <time.h>
int flag = 1;
void shandler_1(int signum)
{
    flag = 0;
}

int main(int argc, char* argv[])
{
    signal (SIGUSR1, shandler_1);
    unlink(argv[1]);
    int fd = open(argv[1], O_WRONLY | O_CREAT, 0666);
    char pathname1[] = "main.c", pathname2[] = "hand.c";
    key_t key1 = ftok(pathname1, 0), key2 = ftok(pathname2, 0);
    pid_t pid = getpid();
    sem_t *sem1 = sem_open("sem1", 0);
    sem_t *sem2 = sem_open("sem2", 0);
    assert(key1 > 0);
    assert(key2 > 0);
    int id_key1 = shmget(key1, sizeof(int), 0666 | IPC_CREAT);
    assert(id_key1 > 0);
    int* sh_pid = (int*)shmat(id_key1, NULL, 0);
    sh_pid[0] = pid;
    shmdt(sh_pid);
    sem_post(sem1);
    int id_key2 = shmget(key2, 1, 0666 | IPC_CREAT);
    assert(id_key2 > 0);
    char* sh_str = (char*)shmat(id_key2, NULL, 0);
    sem_wait(sem2);
    while (flag)
    {
        printf("%c", sh_str[0]);
        write(fd, sh_str, 1);
        sem_post(sem1);
        sem_wait(sem2);
    }
    shmdt(sh_str);
    sem_post(sem1);
    close(fd);
    printf("KILLED\n");
    return 0;
}