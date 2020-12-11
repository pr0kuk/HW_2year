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
    f1.l_len = 0;
    f1.l_pid = 0;
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
        perror("fcntl, setlk");
        return -1;
    }
    sleep(1000);
    close(fd);
    return 0;
}