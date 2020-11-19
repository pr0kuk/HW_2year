#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>


struct stack_t
{
    int id_key;
    int wait;
    void** sh_stack;
    struct timespec timeout;
    sem_t* size;
    sem_t* count;
    sem_t* sem;
};

struct stack_t* attach_stack(key_t key, int size)
{
    struct stack_t* stack = malloc(sizeof(stack_t));
    stack->count = sem_open("count", O_RDWR | O_CREAT, 0666, 0);
    stack->size  = sem_open("size",  O_RDWR | O_CREAT, 0666, size);
    stack->sem   = sem_open("sem",   O_RDWR | O_CREAT, 0666, 1);
    stack->id_key = shmget(key, size, 0);
    stack->wait = -1;
    stack->timeout.tv_sec = 0;
    stack->timeout.tv_nsec = 0;
    if (stack->id_key <= 0)
        stack->id_key = shmget(key, size, 0666 | IPC_CREAT);
    stack->sh_stack = (void**)shmat(stack->id_key, NULL, 0);
    printf("key is %d\n", stack->id_key);
    if (stack->id_key <= 0 || stack->sh_stack <= 0)
        return NULL;
    return stack; 
}

int detach_stack(struct stack_t* stack)
{
    int ret = shmdt(stack->sh_stack);
    free(stack);
    return ret;
}

int mark_destruct(struct stack_t* stack)
{
    shmdt(stack->sh_stack);
    int ret = shmctl(stack->id_key, IPC_RMID, 0);
    printf("stack with return %d have been destructed\n", ret);
    sem_unlink("count");
    sem_unlink("size");
    sem_unlink("sem");
    free(stack);
    return ret;
}

int get_size(struct stack_t* stack)
{
    int ret[1];
    sem_getvalue(stack->size, ret);
    return ret[0];
}

int get_count(struct stack_t* stack)
{
    int ret[1];
    sem_getvalue(stack->count, ret);
    return ret[0];
}

int push(struct stack_t* stack, void* val)
{
    int ret[1];
    int size[1];
    sem_getvalue(stack->size, size);
    int semret[1];
    sem_getvalue(stack->sem, semret);
    sem_wait(stack->sem);
    sem_getvalue(stack->count, ret);
    if (ret[0] == size[0])
    {
        if (stack->wait == -1)
        {   
            sem_post(stack->sem);
            return -1;
        }
        if (stack->wait == 0)
        {
            printf("stack is overflowed, wait flag is 0, waiting infinetely...\n");
            while (ret[0] == size[0])
                sem_getvalue(stack->count, ret);
        }
        if (stack->wait == 1)
        {
            printf("stack is overflowed, wait flag is 1, waiting for %ld seconds and %ld nanoseconds\n", stack->timeout.tv_sec, stack->timeout.tv_nsec);
            nanosleep(&stack->timeout, NULL);
            printf("timeout is ended, ");
            sem_getvalue(stack->count, ret);
            if (ret[0] == size[0])
            {
                sem_post(stack->sem);
                return -1;
            }
        }
    }
    stack->sh_stack[ret[0]] = val;
    sem_post(stack->count);
    sem_post(stack->sem);
    return 0;
}

int pop(struct stack_t* stack, void** val)
{
    int ret[1];
    sem_wait(stack->sem);
    sem_getvalue(stack->count, ret);
    if (ret[0] == 0)
    {
        if (stack->wait == -1)
        {   
            sem_post(stack->sem);
            return -1;
        }
        if (stack->wait == 0)
        {
            printf("stack is empty, wait flag is 0, waiting infinetely...\n");
            while (ret[0] == 0)
                sem_getvalue(stack->count, ret);
        }
        if (stack->wait == 1)
        {
            printf("stack is empty, wait flag is 1, waiting for %ld seconds and %ld nanoseconds\n", stack->timeout.tv_sec, stack->timeout.tv_nsec);
            nanosleep(&stack->timeout, NULL);
            printf("timeout is ended, ");
            sem_getvalue(stack->count, ret);
            if (ret[0] == 0)
            {
                sem_post(stack->sem);
                return -1;
            }
        }
    }
    val[0] = stack->sh_stack[ret[0]-1];
    sem_wait(stack->count);
    sem_post(stack->sem);
    return 0;
}
int set_wait(struct stack_t* stack, int val, struct timespec* timeout)
{
    stack->wait = val;
    if (val == 1)
        stack->timeout = *timeout;
    return 0;
}
int print_all(struct stack_t* stack)
{
    int i = 0;
    int ret[1];
    int size[1];
    sem_getvalue(stack->size, size);
    sem_getvalue(stack->count, ret);
    sem_wait(stack->sem);
    printf("size is %d, count is %d, ", size[0], ret[0]);
    printf("stack is {");
    for (i = 0; i < ret[0]; i++)
    {
        printf("%p", stack->sh_stack[i]);
        if (i != ret[0] - 1)
            printf(", ");
    }
    printf("}\n");
    sem_post(stack->sem);
    return 0;
}
