#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
int main(int argc, char** argv)
{
    int fd, ret = 0;
    struct flock f1 = {0};
    if (argc < 2)
    {
        printf("argc < 2\n");
        return -1;
    }
    fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }
    while (fcntl(fd, F_GETLK, &f1) >= 0)
    {
        if (f1.l_type == F_WRLCK)
            printf("l_type = F_WRLCK, ");
        if (f1.l_type == F_RDLCK) 
            printf("l_type = F_RDLCK, ");
        if (f1.l_type == F_UNLCK)
            printf("l_type = F_UNLCK\n");
        if (f1.l_type != F_UNLCK)
            printf("l_whence=%d, l_len=%ld\n", f1.l_whence, f1.l_len);
    }
    close(fd);
    return 0;
}