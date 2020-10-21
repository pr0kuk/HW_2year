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
#define close_pipes for(i = 0; i < n; i++) close(data_pipe[i][0]), close(data_pipe[i][1]);


int main(int argc, char *argv[])
{
    int data_pipe[1000][2], file = 0, len = 0, i = 0, n = 0, j = -1;
    struct stat fstatbuf;

    if (argc < 2)
    {
        printf("argc < 2\n");
        return 0;
    }

    file = open(argv[1], O_RDONLY);
    fstat(file, &fstatbuf);
    if (file < 0 || ((fstatbuf.st_mode & S_IFMT) != S_IFREG))
    {
        printf("File is not regular\n");
        return 0;
    }

    char* file_str = (char*)malloc(fstatbuf.st_size-1);
    char *substr = NULL;
    pid_t parent = 1;
    read(file, file_str, fstatbuf.st_size-1);
    for (i = 0; i < fstatbuf.st_size-1; i++)
        if (file_str[i] == '|')
            n++;
    for (i = 0; i < n; i++)
        pipe(data_pipe[i]);

    if (parent)
    {
        for (j = 0; substr = strtok_r(file_str, "|", &file_str); j++) 
        {
            parent = fork();
            if (!parent)
                break;
        }
    }

    if (!parent)
    {
        len = strlen(substr);
        char** comms = (char**)malloc(len);
        for (i = 0; comms[i] = strtok_r(substr, " ", &substr); i++)
        if (j > 0)
            dup2(data_pipe[j-1][0], 0);
        if (j < n)
            dup2(data_pipe[j][1], 1);
        close_pipes;
        execvp(comms[0], comms);
    }

    close_pipes;
    for (i = 0; i < n + 1; i++)
        wait(0);
    return 0;
}
