#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
    int in, watch, ret;
    int mask = IN_CREATE | IN_DELETE | IN_MOVE_SELF;
    char buf[4096];
    struct inotify_event* event;
    in = inotify_init();
    if (in < 0)
    {
        printf("inotify was not initialized\n");
        return -1;
    }
    watch = inotify_add_watch(in, argv[1], mask);
    if (watch < 0)
    {
        printf("watch was not added\n");
        return -1;
    }
    
    while (read(in, buf, 4095)>0)
    {
        event = buf;
        if (event->mask & IN_CREATE)
            printf("create %s detected\n", event->name);
        else
        {
            if (event->mask & IN_DELETE)
                printf("delete %s detected\n", event->name);
            else if (event->mask & IN_MOVE_SELF)
                printf("move %s detected\n", event->name);
        }
    }
    return 0;
}