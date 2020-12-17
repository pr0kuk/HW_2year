#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
int main(int argc, char** argv)
{
    int fd, ret;
    struct flock f1 = {0};
    f1.l_type = F_WRLCK;
    f1.l_whence = SEEK_SET;
    f1.l_start = 1;
    f1.l_len = 1;
    f1.l_pid = 0;
    
    struct flock f2 = {0};
    f2.l_type = F_WRLCK;
    f2.l_whence = SEEK_SET;
    f2.l_start = 5;
    f2.l_len = 2;
    f2.l_pid = 0;
    if (argc < 2)
    {
        printf("argc < 2");
        return -1;
    }
    fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }
    ret = fcntl(fd, F_SETLK, &f1);
    if (ret < 0)
    {
        perror("fcntl 1, setlk");
        return -1;
    }
    ret = fcntl(fd, F_SETLK, &f2);
    if (ret < 0)
    {
        perror("fcntl 2, setlk");
        return -1;
    }
    sleep(1000);
    close(fd);
    return 0;
}