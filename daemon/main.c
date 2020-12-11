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
char** wds;
int in = 0;
char** backs;
char* path_argv1;
char* path_argv2;
FILE* fd;

#define LOG(expr, ...)  \
    do { \
    fd = fopen("/home/alexshch/log", "a"); \
    fprintf(fd, expr, __VA_ARGS__); \
    fflush(fd); \
    fclose(fd); \
    } while(0)

int monitor_directory(char * path1, char * path2, int in);
int copy(char * path1, char * path2, uint32_t len, char * name);
int monitor_events(int in);

void kill_all(int signum)
{
    LOG("%d daemon suspended\n", getpid());
    if (wds != NULL)
        free(wds);
    if (backs != NULL)
        free(backs);
    killpg(0, SIGKILL);
}

void change_backup(int signum)
{
    LOG("%d signal SIGUSR1 accepted\n", getpid());
    close(in);
    int id_key = shmget(24, 1, IPC_EXCL);
    char* new_dir = (char*)shmat(id_key, NULL, 0);
    strcpy(path_argv2, new_dir);
    shmctl(id_key, IPC_RMID, NULL);
    LOG("directory changed, new backup directory: %s\n", path_argv2);
    in = inotify_init();
    monitor_directory(path_argv1, path_argv2, in);
    monitor_events(in);
}

void change_work(int signum)
{
    LOG("%d signal SIGUSR2 accepted\n", getpid());
    close(in);
    int id_key = shmget(28, 1, IPC_EXCL);
    char* new_dir = (char*)shmat(id_key, NULL, 0);
    strcpy(path_argv1, new_dir);
    shmctl(id_key, IPC_RMID, NULL);
    LOG("directory changed, new working directory: %s\n", path_argv1);
    in = inotify_init();
    monitor_directory(path_argv1, path_argv2, in);
    monitor_events(in);
}
int copy(char * path1, char * path2, uint32_t len, char * name)
{
    char * buf = (char*)malloc(1);
    /*char* pathcp = (char*)malloc(strlen(path1)+len+15+strlen(path2));
    char* pathgzip = (char*)malloc(strlen(path2)+len+15);
    if (pathcp == NULL)
    {
        LOG("malloc, copy path11, %s\n", strerror(errno));
    }
    if (pathgzip == NULL)
    {
        LOG("malloc, copy path22, %s\n", strerror(errno));
    }
    strcpy(pathcp, "cp -u ");
    strcat(pathcp, path1);
    strcat(pathcp, "/");
    strcat(pathcp, name);
    strcat(pathcp, " ");
    strcat(pathcp, path2);
    system(pathcp);
    LOG("%s\n", pathcp);
    free(pathcp);*/
    char* path11 = (char*)malloc(strlen(path1)+len+15);
    char* path22 = (char*)malloc(strlen(path2)+len+15);
    if (path11 == NULL)
    {
        LOG("malloc, copy path11, %s\n", strerror(errno));
    }
    if (path22 == NULL)
    {
        LOG("malloc, copy path22, %s\n", strerror(errno));
    }
    strcpy(path11, path1);
    strcat(path11, "/");
    strcat(path11, name);
    strcpy(path22, path2);
    strcat(path22, "/");
    strcat(path22, name);
    int fd1 = open(path11, O_RDONLY);
    int fd2 = open(path22, O_WRONLY | O_CREAT, 0666);
    while (read(fd1, buf, 1) > 0)
        write(fd2, buf, 1);
    LOG("%s -> %s\n", path11, path22);
    /*strcpy(pathgzip, "gzip -f ");
    strcat(pathgzip, path2);
    strcat(pathgzip, "/");
    strcat(pathgzip, name);
    system(pathgzip);
    LOG("%s\n", pathgzip);
    free(pathgzip);*/
    free(path11);
    free(path22);
    close(fd1);
    close(fd2);
    return 0;
}

int add_inotify(char * path1, char* path2, int in)
{
    int mask = IN_CREATE | IN_MODIFY | IN_MOVED_TO;
    int wd_cur = inotify_add_watch(in, path1, mask);
    if (wd_cur == -1)
    {
        LOG("%s inotify_add_watch %s\n", path1, strerror(errno));
        return -1;
    }
    wds[wd_cur] = (char*)malloc(strlen(path1)+1);
    backs[wd_cur] = (char*)malloc(strlen(path2)+1);
    if (strcpy(wds[wd_cur], path1) == NULL)
    {
        LOG("strcpy path1=%s, wd_cur=%d add_inotify: %s\n", path1, wd_cur, strerror(errno));
    }
    if (strcpy(backs[wd_cur], path2) == NULL)
    {
        LOG("strcpy path2=%s, wd_cur=%d add_inotify: %s\n", path2, wd_cur, strerror(errno));
    }
    LOG("%s (wd=%d) inotify was initialized\n", path1, wd_cur);
    return 0;
}


int monitor_events(int in)
{
    int lenread;
    char buf[4096];
    char * ptr;
    struct inotify_event* event;
    while (1)
    {
        lenread = read(in, buf, sizeof(buf));
        if (lenread == -1)
        {
            LOG("read, monitor_events, %s\n", strerror(errno));
        }
        if (lenread == 0)
            break;
        for (ptr = buf; ptr < buf + lenread; ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (struct inotify_event *) ptr;
            char* check = (char*)malloc(strlen(wds[event->wd])+event->len+5);
            if (check == NULL)
            {
                LOG("size=%ld malloc check: %s", strlen(wds[event->wd])+event->len+5, strerror(errno));
            }
            strcpy(check, wds[event->wd]);
            strcat(check, "/");
            strcat(check, event->name);
            if (strcmp(check, path_argv2) == 0)
            {
                LOG("monitor_events detected backup dir %s\n", check);
                continue;
            }
            if (check != NULL)
                free(check);
            if (event->mask & IN_CREATE)
            {
                char* path11 = (char*)malloc(strlen(wds[event->wd]) + event->len + 5);
                if (path11 == NULL)
                {
                    LOG("malloc, monitor_events path11, %s\n", strerror(errno));
                }
                strcpy(path11, wds[event->wd]);
                strcat(path11, "/");
                strcat(path11, event->name);
                LOG("created %s\n", path11);
                DIR * dir = opendir(path11);
                if (dir == NULL)
                {
                    copy(wds[event->wd], backs[event->wd], event->len, event->name);
                }
                else
                {   
                    char* path22 = (char*)malloc(strlen(wds[event->wd]) + event->len + 5);
                    if (path22 == NULL)
                    {
                        LOG("malloc, monitor_events path22, %s\n", strerror(errno));
                    }
                    strcpy(path22, backs[event->wd]);
                    strcat(path22, "/");
                    strcat(path22, event->name);
                    closedir(dir);
                    monitor_directory(path11, path22, in);
                    //if (path22 != NULL)
                    //    free(path22);
                }
                //if (path11 != NULL)
                //    free(path11);
            }
            if (event->mask & IN_MODIFY)
            {
                LOG("modified %s/%s\n", wds[event->wd], event->name);
                copy(wds[event->wd], backs[event->wd], event->len, event->name);
            }
            if (event->mask & IN_MOVED_TO)
            {
                LOG("renamed %s/%s\n", wds[event->wd], event->name);
                char* path11 = (char*)malloc(strlen(wds[event->wd]) + event->len + 5);
                if (path11 == NULL)
                {
                    LOG("malloc, monitor_events path11, %s\n", strerror(errno));
                }
                strcpy(path11, wds[event->wd]);
                strcat(path11, "/");
                strcat(path11, event->name);
                LOG("moved_to dir %s\n", path11);
                DIR * dir = opendir(path11);
                if (dir == NULL)
                {
                    copy(wds[event->wd], backs[event->wd], event->len, event->name);
                }
                else
                {   
                    char* path22 = (char*)malloc(strlen(wds[event->wd]) + event->len + 5);
                    if (path22 == NULL)
                    {
                        LOG("malloc, monitor_events path22, %s\n", strerror(errno));
                    }
                    strcpy(path22, backs[event->wd]);
                    strcat(path22, "/");
                    strcat(path22, event->name);
                    closedir(dir);
                    monitor_directory(path11, path22, in);
                    /*if (path22 != NULL)
                        free(path22);*/
                }
            }
        }
    }
    LOG("%d inotify was stopped\n", getpid());
    return 0;
}

int monitor_directory(char * path1, char * path2, int in)
{
    struct dirent* pointer;
    if (strcmp(path1, path_argv2) == 0)
    {
        LOG("monitor_directory detected backup dir %s\n", path1);
        return -1;
    }
    DIR * dir = opendir(path1);
    if(dir == NULL)
    {
        LOG("%s monitor_directory error %s\n", path1, strerror(errno));
        return -1;
    }
    if (mkdir(path2, 0777) == -1)
    {
        LOG("mkdir: %s\n", strerror(errno));
    }
    LOG("mkdir %s\n", path2);
    add_inotify(path1, path2, in);
    while(1)
    {
        pointer = readdir(dir);
        if (pointer == NULL)
            break;
        if ((strcmp(pointer->d_name, ".") == 0) || (strcmp(pointer->d_name, "..") == 0))
            continue;
        if (pointer->d_type == DT_DIR)
        {   
            char* path11 = (char*)malloc(strlen(path1) + strlen(pointer->d_name) + 5);
            char* path22 = (char*)malloc(strlen(path2) + strlen(pointer->d_name) + 5);
            if (path11 == NULL)
            {
                LOG("malloc, copy path11, %s\n", strerror(errno));
            }
            if (path22 == NULL)
            {
                LOG("malloc, copy path22, %s\n", strerror(errno));
            }
            strcpy(path11, path1);
            strcpy(path22, path2);
            strcat(path11, "/");
            strcat(path22, "/");
            strcat(path11, pointer->d_name);
            strcat(path22, pointer->d_name);
            monitor_directory(path11, path22, in);
            /*if (path11 != NULL)
                free(path11);
            if (path22 != NULL)
                free(path22);*/
        }
        else
        {
            copy(path1, path2, strlen(pointer->d_name), pointer->d_name);
        }
    }
    closedir(dir);
    return 0;
}

int main(int argc, char * argv[])
{
    signal(SIGINT, kill_all);
    signal(SIGUSR1, change_backup);
    signal(SIGUSR2, change_work);
    if (argc < 3)
    {
        printf("argc < 3\n");
        return -1;
    }
    path_argv1 = (char*)malloc(100);
    path_argv2 = (char*)malloc(100);
    strcpy(path_argv1, argv[1]);
    strcpy(path_argv2, argv[2]);
    if (strcmp(path_argv1, path_argv2) == 0)
    {
        LOG("%s %s argv[1]==argv[2]\n", path_argv1, path_argv2);
        return -1;
    }
    wds = (char**)calloc(100, sizeof(char*));
    backs = (char**)calloc(100, sizeof(char*));
    if (daemon(1, 1) == -1)
    {
        perror("daemon");
        return -1;
    }
    int id_key = shmget(42, sizeof(unsigned int), IPC_CREAT | 0666);
    pid_t* pid = (pid_t*)shmat(id_key, NULL, 0);
    *pid = getpid();
    LOG("current backup directory is %s\n", path_argv2);
    LOG("current working directory is %s\n",path_argv1);
    LOG("main pid is %d\n", getpid());
    in = inotify_init();
    if (in == -1)
    {
        LOG("inotify_init: %s\n", strerror(errno));
        return -1;
    }
    monitor_directory(path_argv1, path_argv2, in);
    monitor_events(in);
    return 0;
}