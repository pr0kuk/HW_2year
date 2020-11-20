#include "stack.h"
int main()
{
    struct stack_t* st;
    key_t key = ftok("stack.h", 0);
    int i = 0;
    int size = (2 << 9)*sizeof(void*);
    pid_t main_pid = getpid();
    char *  z   = "1";
    void *  zz  = (void*)z;
    void ** out = (void**)malloc(sizeof(void*));
    setgid(main_pid);
    for (i = 0; i < 5; i++)
        fork();
    st = attach_stack(key, size);
    if (st == NULL)
        printf("error\n");
    for (i = 0; i < 10; i++)
        push(st, zz);
    for (i = 0; i < 10; i++)
        pop(st, out);
    if (getpid() == main_pid)
    {
        setgid(main_pid+1);
        sleep(1);
        killpg(main_pid, SIGKILL);
        push(st, zz);
        push(st, zz);
        pop(st, out);
        pop(st, out);
        print_all(st);
        mark_destruct(st);
    }
    else
        detach_stack(st);
    return 0;
}