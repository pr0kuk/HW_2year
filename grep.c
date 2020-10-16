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

int main(int argc, char *argv[])
{
    int data_pipe[2], file = 0, len = 0, num = 0, pip = -1, pip2 = -1, bt = 0;
    char buf[1] = "0";
    char ch;
    struct stat fstatbuf, stpip, stpip2;
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
    int i = 0;
    char* file_str = (char*)malloc(fstatbuf.st_size-1);
    read(file, file_str, fstatbuf.st_size-1);
    i = 0;
    char *command = NULL;
    char *remaining = file_str;
    pid_t child = 1;
    pipe(data_pipe);

    if (child != 0)
    {
        while (command = strtok_r(remaining, "|", &remaining))
        {
            child = fork();
            if (child == 0)
                break;
            wait(0);
        }
    }

    
    if (child == 0)
    {
        while (1)
        {
            dup2(data_pipe[0], 0);
            dup2(data_pipe[1], 1);
            int len = strlen(command), i = 0;
            if (len == 0)
                break;
            char** comms = (char**)malloc(len);
            while(comms[i] = strtok_r(command, " ", &command))
                i++;
            execvp(comms[0], comms);
        }
    }
    int size = 1;
    while((read(data_pipe[0], buf, 1)) > 0)
        printf("%s", buf);
    return 0;
}
