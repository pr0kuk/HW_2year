#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
int main(int argc, char* argv[])
{
    DIR * dir = opendir(argv[1]);
    if (dir == NULL)
    {
        printf("Invalid directory\n");
        return -1;
    }
    struct  dirent* pointer = readdir(dir);
    while(pointer != NULL)
    {
        printf("%s\n", pointer->d_name);
        pointer = readdir(dir);
    }
    return 0;
}