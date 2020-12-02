#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

void copy(char * path1, char * path2, uint32_t len, char * name)
{
    char * divisor = "/";
    char * path11 = (char*)malloc(strlen(path1) + len);
    char * path22 = (char*)malloc(strlen(path1) + len);
    strcpy(path11, path1);
    strcpy(path22, path2);
    strcat(path11, divisor);
    strcat(path22, divisor);
    strcat(path11, name);
    strcat(path22, name);
    printf("%s\n%s\n", path11, path22);
    pid_t pid = fork();
    if (pid == 0)
        execlp("cp", "cp", path11, path22, NULL);
}

int main(int argc, char * argv[])
{
    int in, watch, lenread = 0;
    int mask = IN_CREATE | IN_MODIFY | IN_MOVED_TO;
    if (argc < 3)
    {
        printf("argc < 3\n");
        return -1;
    }
    char buf[4096];
    char * ptr;
    struct inotify_event* event;
    in = inotify_init();
    if (in == -1)
    {
        perror("inotify_init");
        return -1;
    }
    watch = inotify_add_watch(in, argv[1], mask);
    if (watch == -1)
    {
        perror("inotify_add_watch");
        return -1;
    }
    pid_t pid = fork();
    char* path11 = (char*)malloc(strlen(argv[1])+2);
    strcpy(path11, argv[1]);
    strcat(path11, "/.");
    if (pid == 0)
        execlp("cp", "cp", "-r", "-u", "-v", path11, argv[2], NULL);
    printf("inotify was initialized\n");
    while (1)
    {
        lenread = read(in, buf, sizeof(buf));
        if (lenread == -1)
        {
            perror("read");
            return -1;
        }
        if (lenread == 0)
            break;
        for (ptr = buf; ptr < buf + lenread; ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (const struct inotify_event *) ptr;
            if (event->mask & IN_CREATE)
            {
                //printf("creat\n");
                copy(argv[1], argv[2], event->len, event->name);
                //printf("creating %s was detected\n", event->name);
            }
            if (event->mask & IN_MODIFY)
            {
                //printf("modif\n");
                copy(argv[1], argv[2], event->len, event->name);
                //printf("deleting %s was detected\n", event->name);
            }
            if (event->mask & IN_MOVED_TO)
            {
                //printf("moved\n");
                copy(argv[1], argv[2], event->len, event->name);
            }

        }
    }
    return 0;
}