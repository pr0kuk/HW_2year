#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>


struct stack_t
{
    int id_key;
    int size;
    sem_t *count;
    sem_t *sem;
    void** sh_stack;
};

struct stack_t* attach_stack(key_t key, int size)
{
    struct stack_t* stack = malloc(sizeof(stack_t));
    sem_unlink("sem");
    stack->count = sem_open("count", O_RDWR | O_CREAT, 0666, 0);
    stack->sem = sem_open("sem", O_RDWR | O_CREAT, 0666, 1);
    stack->id_key = shmget(key, size, 0666 | IPC_CREAT);
    stack->sh_stack = (void**)shmat(stack->id_key, NULL, 0);
    if (stack->id_key <= 0 || stack->sh_stack <= 0)
        return NULL;
    stack->size = size;
    return stack; 
}

int detach_stack(struct stack_t* stack)
{
    shmdt(stack->sh_stack);
    free(stack);
    return 0;
}

int mark_destruct(struct stack_t* stack)
{
    shmdt(stack->sh_stack);
    printf("stack with key %d have been destructed\n", shmctl(stack->id_key, IPC_RMID, 0));
    sem_unlink("count");
    sem_unlink("sem");
    free(stack);
    return 0;
}

int get_size(struct stack_t* stack)
{
    return stack->size;
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
    int semret[1];
    sem_getvalue(stack->sem, semret);
    sem_wait(stack->sem);
    sem_getvalue(stack->count, ret);
    if (ret[0] == (stack->size-1))
        return -1;
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
        return -1;
    val[0] = stack->sh_stack[ret[0]-1];
    sem_wait(stack->count);
    sem_post(stack->sem);
    return 0;
}

int print_all(struct stack_t* stack)
{
    int i = 0;
    int ret[1];
    sem_getvalue(stack->count, ret);
    sem_wait(stack->sem);
    printf("size is %d, count is %d, ", stack->size, ret[0]);
    printf("stack is { ");
    for (i = 0; i < ret[0]; i++)
    {
        printf("%p, ", stack->sh_stack[i]);
    }
    printf("}\n");
    sem_post(stack->sem);
    return 0;
}
