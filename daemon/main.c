#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
int monitor_directory(char * path1, char * path2);
int copy(char * path1, char * path2, uint32_t len, char * name);
int monitor_events(char * path1, char * path2);
void kill_all(int signum)
{
    killpg(0, SIGKILL);
}


int copy(char * path1, char * path2, uint32_t len, char * name)
{
    char * path11 = (char*)malloc(strlen(path1) + len + 1);
    char * path22 = (char*)malloc(strlen(path1) + len + 1);
    if (path11 == NULL)
    {
        printf("copy error %s\n", path1);
        perror("malloc");
        return -1;
    }
    if (path22 == NULL)
    {
        printf("copy error %s\n", path2);
        perror("malloc");
        return -1;
    }
    strcpy(path11, path1);
    strcpy(path22, path2);
    strcat(path11, "/");
    strcat(path22, "/");
    strcat(path11, name);
    strcat(path22, name);
    printf("%d %s %s\n", getpid(), path11, path22);
    if (fork() == 0)
        execlp("cp", "cp", path11, path22, NULL);
    return 0;
}


int monitor_events(char * path1, char * path2)
{
    int in, watch, lenread;
    int mask = IN_CREATE | IN_MODIFY | IN_MOVED_TO;
    char buf[4096];
    char * ptr;
    struct inotify_event* event;
    in = inotify_init();
    if (in == -1)
    {
        perror("inotify_init");
        return -1;
    }
    watch = inotify_add_watch(in, path1, mask);
    if (watch == -1)
    {
        perror("inotify_add_watch");
        return -1;
    }
    printf("%d %s inotify was initialized\n", getpid(), path1);
    if (fork() == 0)
    {
        char* path11 = (char*)malloc(strlen(path1)+2);
        if (path11 == NULL)
        {
            printf("monitor events %s\n", path1);
            perror("malloc");
            return -1;
        }
        strcpy(path11, path1);
        strcat(path11, "/.");
        execlp("cp", "cp", "-r", "-u", "-v", "-p", path11, path2, NULL);
    }
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
            event = (struct inotify_event *) ptr;
            if (event->mask & IN_CREATE)
            {
                //printf("creat %s\n", event->name);
                char* path11 = (char*)malloc(strlen(path1) + event->len + 1);
                if (path11 == NULL)
                {
                    //printf("%s %s\n", path1, pointer->d_name);
                    perror("event malloc");
                    return -1;
                }
                strcpy(path11, path1);
                strcat(path11, "/");
                strcat(path11, event->name);
                DIR * dir = opendir(path11);
                if (dir == NULL)
                    copy(path1, path2, event->len, event->name);
                else
                {   
                    if (fork()==0)
                    {
                        char* path22 = (char*)malloc(strlen(path2) + event->len + 1);
                        if (path22 == NULL)
                        {
                            perror("event malloc");
                            return -1;
                        }
                        strcpy(path22, path2);
                        strcat(path22, "/");
                        strcat(path22, event->name);
                        monitor_directory(path11, path22);
                    }
                }
                //printf("creating %s was detected\n", event->name);
            }
            if (event->mask & IN_MODIFY)
            {
                //printf("modif\n");
                copy(path1, path2, event->len, event->name);
                //printf("deleting %s was detected\n", event->name);
            }
            if (event->mask & IN_MOVED_TO)
            {
                //printf("moved\n");
                copy(path1, path2, event->len, event->name);
            }

        }
        printf("%d inotify was stopped\n", getpid());
        raise(SIGKILL);
    }
    return 0;
}

int monitor_directory(char * path1, char * path2)
{
    struct dirent* pointer;
    char * path11;
    char * path22;
    DIR * dir = opendir(path1);
    if(dir == NULL)
    {
        perror("opendir");
        return -1;
    }
    while(1)
    {
        pointer = readdir(dir);
        if (pointer == NULL)
            break;
        if ((strcmp(pointer->d_name, ".") == 0) || (strcmp(pointer->d_name, "..") == 0))
            continue;
        if (pointer->d_type == DT_DIR)
        {   
            path11 = (char*)malloc(strlen(path1) + strlen(pointer->d_name) + 2);
            path22 = (char*)malloc(strlen(path2) + strlen(pointer->d_name) + 2);
            if (path11 == NULL)
            {
                perror("malloc");
                return -1;
            }
            if (path22 == NULL)
            {
                perror("malloc");
                return -1;
            }
            strcpy(path11, path1);
            strcpy(path22, path2);
            strcat(path11, "/");
            strcat(path22, "/");
            strcat(path11, pointer->d_name);
            strcat(path22, pointer->d_name);
            if (fork() == 0)
                monitor_directory(path11, path22);
        }
    }
    closedir(dir);
    monitor_events(path1, path2);
    return 0;
}



int main(int argc, char * argv[])
{
    signal (SIGINT, kill_all);
    printf("main pid is %d\n", getpid());
    if (argc < 3)
    {
        printf("argc < 3\n");
        return -1;
    }
    if (fork() == 0)
        monitor_directory(argv[1], argv[2]);
    pause();
    return 0;
}