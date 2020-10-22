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
#include <time.h>
int main(int argc, char *argv[])
{
    int c, i;
    //clock_t start = clock();
    int mas[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    char buf[1];
    struct stat fstatbuf;
    if (argc < 3)
    {
        printf("argc < 3\n");
        return 0;
    }
    int pid = atoi(argv[2]);
    int file = open(argv[1], O_RDONLY);
    fstat(file, &fstatbuf);
    if (file < 0 || ((fstatbuf.st_mode & S_IFMT) != S_IFREG))
    {
        printf("File is not regular\n");
        return 0;
    }
    while (read(file, buf, 1) > 0)
    {
        c = (int)buf[0];
        //printf("%d ", c);
        for (i = 0; i < 8; i++)
        {
            if ((c&1) == 0)
                kill(pid, SIGUSR1);
            else
                kill(pid, SIGUSR2);
            usleep(10);
            c = c >> 1;
        }
    }
    kill(pid, SIGTERM);
    //clock_t end = clock();
    //printf("time of exectuing is %lf seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    return 0;
}