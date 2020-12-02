#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
    int in, watch, lenread = 0;
    int mask = IN_CREATE | IN_DELETE | IN_MOVE_SELF;
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
                printf("creating %s was detected\n", event->name);
            if (event->mask & IN_DELETE)
                printf("deleting %s was detected\n", event->name);
            if (event->mask & IN_MOVE_SELF)
                printf("moving %s was detected\n", argv[1]);
        }
    }
    return 0;
}