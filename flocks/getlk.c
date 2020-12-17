#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
int main(int argc, char** argv)
{
    int fd, ret = 0, i = 0, flag = 0;
    struct flock f1 = {0};
    f1.l_type = 0;
    f1.l_whence = SEEK_SET;
    f1.l_start = 0;
    f1.l_len = 0;
    f1.l_pid = 0;
    struct flock f2 = {0};
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
    struct stat fstatbuf;
    fstat(fd, &fstatbuf);
    size_t len = fstatbuf.st_size;
    while (i < len)
    {
        //printf("start is %ld\n", f1.l_start);
        if (fcntl(fd, F_GETLK, &f1) >= 0 && f1.l_type != F_UNLCK)
        {
            if (f1.l_type == F_WRLCK)
                printf("l_type = F_WRLCK, ");
            if (f1.l_type == F_RDLCK) 
                printf("l_type = F_RDLCK, ");
            printf("l_whence=%d, l_start=%ld, l_len=%ld, l_pid=%d\n", f1.l_whence, f1.l_start, f1.l_len, f1.l_pid);
            flag = 1;
            i = f1.l_start + f1.l_len;
            f1.l_len = 0;
            f1.l_whence = SEEK_SET;
            f1.l_pid = 0;
            f1.l_start = i;
        }
        else
            i++, f1.l_start++;
    }
    if (!flag)
        printf("l_type = F_UNLCK\n");
    close(fd);
    return 0;
}