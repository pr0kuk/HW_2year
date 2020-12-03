#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

char* path_argv1;
char* path_argv2;
FILE* fd;
#define LOG(expr, ...)  \
    fd = fopen("/home/alexshch/HW_2year/daemon/log", "a"); \
    fprintf(fd, expr, __VA_ARGS__); \
    fflush(fd); \
    fclose(fd);

int monitor_directory(char * path1, char * path2);
int copy(char * path1, char * path2, uint32_t len, char * name);
int monitor_events(char * path1, char * path2);

void kill_all(int signum)
{
    LOG("%d daemon suspended\n", getpid());
    killpg(0, SIGKILL);
}

void change(int signum)
{
    int id_key = shmget(24, strlen(path_argv1), IPC_CREAT | 0666);
    char* new_dir = (char*)shmat(id_key, NULL, 0);
    strcpy(new_dir, path_argv1);
    LOG("directory changed, old directory: %s\n", path_argv2);
    raise(SIGINT);
}

int copy(char * path1, char * path2, uint32_t len, char * name)
{
    char * path11 = (char*)malloc(strlen(path1) + len + 1);
    char * path22 = (char*)malloc(strlen(path1) + len + 1);
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
    strcat(path11, name);
    strcat(path22, name);
    if (fork() == 0)
    {
        int fder = open("/home/alexshch/HW_2year/daemon/log2", O_WRONLY | O_APPEND | O_TRUNC | O_CREAT | O_SYNC, 0666);
        dup2(fder, 2);
        pid_t child = fork();
        if (child == 0)
            execlp("cp", "cp", "-u", path11, path22, NULL);
        waitpid(child, NULL, 0);
        execlp("gzip", "gzip", "-f", path22, NULL);
        LOG("cp error: %s", strerror(errno));
        raise(SIGKILL);
    }
    LOG("%d %s -> %s\n", getpid(), path11, path22);
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
    LOG("%d %s inotify was initialized\n", getpid(), path1);
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
                char* path11 = (char*)malloc(strlen(path1) + event->len + 1);
                if (path11 == NULL)
                {
                    perror("event malloc");
                    return -1;
                }
                strcpy(path11, path1);
                strcat(path11, "/");
                strcat(path11, event->name);
                LOG("created %s\n", path11);
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
            }
            if (event->mask & IN_MODIFY)
            {
                LOG("modified %s/%s\n", path1, event->name);
                copy(path1, path2, event->len, event->name);
            }
            if (event->mask & IN_MOVED_TO)
            {
                LOG("renamed %s/%s\n", path1, event->name);
                                char* path11 = (char*)malloc(strlen(path1) + event->len + 1);
                if (path11 == NULL)
                {
                    perror("event malloc");
                    return -1;
                }
                strcpy(path11, path1);
                strcat(path11, "/");
                strcat(path11, event->name);
                LOG("created %s\n", path11);
                DIR * dir = opendir(path11);
                if (dir == NULL)
                    copy(path1, path2, event->len, event->name);
                else
                {   
                    if (fork() == 0)
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
            }
        }
    }
    LOG("%d inotify was stopped\n", getpid());
    raise(SIGKILL);
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
    pid_t child = fork();
    if (child == 0)
    {
        int fder = open("/home/alexshch/HW_2year/daemon/log2", O_WRONLY | O_APPEND | O_TRUNC | O_CREAT | O_SYNC, 0666);
        dup2(fder, 2);
        execlp("mkdir", "mkdir", "-p", path2, NULL);
        LOG("mkdir error: %s", strerror(errno))
        raise(SIGKILL);
    }
    waitpid(child, NULL, 0);
    LOG("created directory %s\n", path2);
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
        else
            copy(path1, path2, strlen(pointer->d_name), pointer->d_name);
    }
    closedir(dir);
    monitor_events(path1, path2);
    return 0;
}

int main(int argc, char * argv[])
{
    signal(SIGINT, kill_all);
    signal(SIGUSR1, change);
    if (argc < 3)
    {
        printf("argc < 3\n");
        return -1;
    }
    path_argv1 = argv[1];
    path_argv2 = argv[2];
    if (daemon(1, 1) == -1)
    {
        perror("daemon");
        return -1;
    }
    int id_key = shmget(42, sizeof(unsigned int), IPC_CREAT | 0666);
    pid_t* pid = (pid_t*)shmat(id_key, NULL, 0);
    *pid = getpid();
    LOG("current directory is %s\n", path_argv2);
    LOG("main pid is %d\n", getpid());
    if (fork() == 0)
        monitor_directory(argv[1], argv[2]);
    while(1)
        pause();
    return 0;
}