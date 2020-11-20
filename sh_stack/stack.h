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
#include <sys/sem.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>


struct stack_t
{
    int id_key;
    int wait;
    int sem;
    void** sh_stack;
    struct timespec timeout;
};

struct stack_t* attach_stack(key_t key, int size)
{
    struct stack_t* stack = malloc(sizeof(struct stack_t));
    stack->id_key = shmget(key, size, IPC_EXCL);
    stack->wait = -1;
    stack->timeout.tv_sec = 0;
    stack->timeout.tv_nsec = 0;
    if (stack->id_key > 0)
    {
        printf("stack has been already created\n");
        stack->id_key = shmget(key, size, 0666 | IPC_CREAT);
        stack->sem = semget(key, 3, IPC_EXCL);
        semctl(stack->sem, 2, SETVAL, 1);
    }
    else
    {
        stack->id_key = shmget(key, size, IPC_CREAT | 0666);
        stack->sem = semget(key, 3, IPC_CREAT | 0666);
        semctl(stack->sem, 0, SETVAL, size);
        semctl(stack->sem, 1, SETVAL, 0);
        semctl(stack->sem, 2, SETVAL, 1);
    }
    stack->sh_stack = (void**)shmat(stack->id_key, NULL, 0);
    printf("size is %d\n", semctl(stack->sem, 0, GETVAL));
    printf("key is %d\n", stack->id_key);
    if (stack->id_key <= 0 || stack->sh_stack <= 0)
        return NULL;
    int size1 = semctl(stack->sem, 0, GETVAL);
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
    semctl(stack->sem, 0, IPC_RMID);
    free(stack);
    return ret;
}

int get_size(struct stack_t* stack)
{
    int ret = semctl(stack->sem, 0, GETVAL);
    return ret;
}

int get_count(struct stack_t* stack)
{
    int ret = semctl(stack->sem, 1, GETVAL);
    return ret;
}

int push(struct stack_t* stack, void* val)
{
    int size = semctl(stack->sem, 0, GETVAL);
    int count = semctl(stack->sem, 1, GETVAL);
    struct sembuf buf;
    buf.sem_num = 2, buf.sem_op = -1, buf.sem_flg = 0;
    if (count == size)
    {
        if (stack->wait == -1)
        {   
            return -1;
        }
        if (stack->wait == 0)
        {
            printf("stack is overflowed, wait flag is 0, waiting infinetely...\n");
            while (count == size)
                count = semctl(stack->sem, 1, GETVAL);
        }
        if (stack->wait == 1)
        {
            printf("stack is overflowed, wait flag is 1, waiting for %ld seconds and %ld nanoseconds\n", stack->timeout.tv_sec, stack->timeout.tv_nsec);
            nanosleep(&stack->timeout, NULL);
            printf("timeout is ended, ");
            count = semctl(stack->sem, 1, GETVAL);
            if (count == size)
            {
                return -1;
            }
        }
    }
    semop(stack->sem, &buf, 1);
    stack->sh_stack[count] = val;
    buf.sem_op = 1, buf.sem_num = 1;
    semop(stack->sem, &buf, 1);
    buf.sem_num = 2;
    semop(stack->sem, &buf, 1);
    return 0;
}

int pop(struct stack_t* stack, void** val)
{
    int count = semctl(stack->sem, 1, GETVAL);
    struct sembuf buf;
    buf.sem_num = 2, buf.sem_op = -1, buf.sem_flg = 0;
    if (count == 0)
    {
        if (stack->wait == -1)
        {   
            return -1;
        }
        if (stack->wait == 0)
        {
            printf("stack is empty, wait flag is 0, waiting infinetely...\n");
            while (count == 0)
                count = semctl(stack->sem, 1, GETVAL);
        }
        if (stack->wait == 1)
        {
            printf("stack is empty, wait flag is 1, waiting for %ld seconds and %ld nanoseconds\n", stack->timeout.tv_sec, stack->timeout.tv_nsec);
            nanosleep(&stack->timeout, NULL);
            printf("timeout is ended, ");
            count = semctl(stack->sem, 1, GETVAL);
            if (count == 0)
            {
                return -1;
            }
        }
    }
    semop(stack->sem, &buf, 1);
    val[0] = stack->sh_stack[count-1];
    buf.sem_op = -1, buf.sem_num = 1;
    semop(stack->sem, &buf, 1);
    buf.sem_op = 1, buf.sem_num = 2;
    semop(stack->sem, &buf, 1);
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
    int size = semctl(stack->sem, 0, GETVAL);
    int count = semctl(stack->sem, 1, GETVAL);
    struct sembuf buf;
    buf.sem_num = 2, buf.sem_op = -1, buf.sem_flg = 0;
    semop(stack->sem, &buf, 1);
    printf("size is %d, count is %d, ", size, count);
    printf("stack is {");
    for (i = 0; i < count; i++)
    {
        printf("%p", stack->sh_stack[i]);
        if (i != count - 1)
            printf(", ");
    }
    printf("}\n");
    buf.sem_op = 1;
    semop(stack->sem, &buf, 1);
    return 0;
}
