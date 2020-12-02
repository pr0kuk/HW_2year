#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
    char buf[1];
    int flag = 0, ret = 0, i = 0, n = argc - 1;
    int * fd = (int*)malloc((argc-1)*sizeof(int));
    int * flags = (int*)malloc((argc-1)*sizeof(int));
    struct pollfd * st = (struct pollfd *)malloc(n * sizeof(struct pollfd));

    for (i = 0; i < n; i++)
        fd[i] = open(argv[i+1], O_RDONLY | O_NONBLOCK);
    for (i = 0; i < n; i++)
    {
        st[i].fd = fd[i];
        st[i].events = POLLIN;
    }

    while(flag != n)
    {
        ret = poll(st, n, 1);
        for (i = 0; i < n; i++)
        {
            if (st[i].events != 0 && st[i].revents !=0)
            {
                printf("file %s\n", argv[i + 1]);
                while(read(fd[i], buf, 1) > 0)
                    write(1, buf, 1);
                printf("\n");
                st[i].events = 0;
                flag++;
            }
        }
    }
    return 0;
}