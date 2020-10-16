#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

void transfer (int fd1, int fd2, int len)
{
    char cur = 0, buf = 0;
    int i = 0, last_sp = -1, summa = 0, flag = 0;
    for (i = 0; i < len; i++)
    {
        read(fd1, &cur, 1);
        if (!isspace(cur) && ((cur != '-') || ((i != last_sp + 1) && (i != 0))) && (cur < 48 || cur > 57) || ((buf == '-') && isspace(cur)))
            flag = 1;
        if ((!isspace(cur) && cur != '-'))
            {buf = cur, summa = (summa + buf - 48) % 3;}
        if ((isspace(cur) && i != last_sp && i != 0 && !isspace(buf)))
        {
            if (summa % 3 == 0 && flag == 0)
                write(fd2, "bizz", 4), flag = 0, summa = 0;
            if ((buf - 48) % 5 == 0 && flag == 0)
                write(fd2, "buzz", 4), flag = 0, summa = 0;
            if (flag == 1 || summa != 0)
            {
                i = last_sp, cur = 0, summa = 0;
                lseek(fd1, last_sp + 1, SEEK_SET);
                for (i++; read(fd1, &cur, 1); i++)
                {
                    if (isspace(cur) && i != last_sp)
                        break;
                    write(fd2, &cur, 1);
                    if (i == len - 1)
                        break;
                }
                flag = 0;
            }
            buf = cur, last_sp = i;
        }
        if (isspace(cur))
            {write(fd2, &cur, 1); last_sp = i;}
        buf = cur;
    }
}

int main(int argc, char** argv)
{
    struct stat fstatbuf1, fstatbuf2;
    if (argc < 3)
        printf("Not enough arguments\n");
    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 < 0 && argc > 2)
            printf("Input file not found\n");
    if (argc > 2 && fd1 >= 0)
    {
        fstat(fd1, &fstatbuf1);
        if ((fstatbuf1.st_mode & S_IFMT) == S_IFREG)
        {
            int fd2 = open(argv[2], O_WRONLY | O_CREAT, 0666);
            fstat (fd2, &fstatbuf2);
            if ((fstatbuf1.st_mode & S_IFMT) == S_IFREG && fd2 >= 0)
            {
                transfer(fd1, fd2, fstatbuf1.st_size);
                close(fd2);
            }
            else
                printf("Output file cannot be open\n");
        }
        else
            printf("Input file is not regular\n");
    }
    close(fd1);
    return 0;
}
