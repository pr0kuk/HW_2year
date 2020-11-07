#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
int main(int argc, char * argv[])
{
    int fd = open(argv[1], O_RDONLY), i = 0;
    struct stat fstatbuf;
    char pathname1[] = "main.c", pathname2[] = "hand.c", pathname3[] = "-hand";
    key_t key1 = ftok(pathname1, 0);
    key_t key2 = ftok(pathname2, 0);
    assert(key1 > 0);
    assert(key2 > 0);
    sem_unlink("sem1");
    sem_unlink("sem2");
    sem_t *sem1 = sem_open("sem1", O_RDWR | O_CREAT, 0666, 0);
    sem_t *sem2 = sem_open("sem2", O_RDWR | O_CREAT, 0666, 0);
    fstat(fd, &fstatbuf);
    int id_key1 = shmget(key1, sizeof(int), 0666 | IPC_CREAT);
    assert(id_key1 > 0);
    sem_wait(sem1);
    int* sh_pid = (int*)shmat(id_key1, NULL, 0);
    pid_t pid = sh_pid[0];
    shmdt(sh_pid);
    shmctl(id_key1, IPC_RMID, 0);
    char buf[1];
    int id_key2 = shmget(key2, 1, 0666 | IPC_CREAT);
    assert(id_key2 > 0);
    char* sh_str = (char*)shmat(id_key2, NULL, 0);
    while (read(fd, buf, 1) > 0)
    {
        printf("%c", buf[0]);
        sh_str[0] = buf[0];
        sem_post(sem2);
        if (i < fstatbuf.st_size - 1)
            sem_wait(sem1);
        i++;
    }
    kill(pid, SIGUSR1);
    printf("END\n");
    sem_post(sem2);
    sem_wait(sem1);
    shmdt(sh_str);
    shmctl(id_key2, IPC_RMID, 0);
    sem_unlink("sem1");
    sem_unlink("sem2");
    close(fd);
    return 0;
}