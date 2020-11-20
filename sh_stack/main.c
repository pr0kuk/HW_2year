#include "stack.h"

struct stack_t* stack;

int handler(FILE * fd)
{
    int ret = 0, wait = 0;
    int w[2];
    void** out   = (void**)malloc(10);
    char*  temp  = (char*)malloc(10);
    char*  input = (char*)malloc(10);
    char*  instr = (char*)malloc(10);
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 0;
    fscanf(fd, "%s", instr);
    if (instr[0] == 'f')
        return 1;
    if (instr[0] == 'g')
    {
        if (instr[4] == 's')
            printf("size is %d\n", get_size(stack));
        if (instr[4] == 'c')
            printf("count is %d\n", get_count(stack));
    }
    if (instr[0] == 's')
    {
        fscanf(fd, "%d", &wait);
        if (wait == 1)
        {
            fscanf(fd, "%d%d", &w[0], &w[1]);
            timeout.tv_sec = w[0];
            timeout.tv_nsec = w[1];
            set_wait(stack, wait, &timeout);
        }
        if (wait == 0 || wait == -1)
            set_wait(stack, wait, NULL);
    }
    if (instr[0] == 'p')
    {
        if (instr[1] == 'u')
        {
            temp = (char*)malloc(10);
            input = temp;
            fscanf(fd, "%s", input);
            void* val = (void*)input;
            ret = push(stack, val);
            if (ret == 0)
                printf("pushed %p %s\n", val, (char*)val);
            if (ret == -1)
                printf("stack is overflowed, %p %s cannot be pushed\n", val, (char*)val);
        }
        if (instr[1] == 'o')
        {           
            ret = pop(stack, out);
            if (ret == 0)
                printf("popped %p\n", *out);
            if (ret == -1)
                printf("stack is empty\n");
        }
        if (instr[1] == 'r')
            print_all(stack);
    }
    if (instr[0] == 'd')
    {
        printf("stack has been detached\n");
        detach_stack(stack);
    }
    if (instr[0] == 'm')
    {
        mark_destruct(stack);
        return 1;
    }
    return 0;
}



int main(int argc, char* argv[])
{
    if (argc >=2) //working with file
    {
        FILE * fd = fopen(argv[1], "r");
        int size = 0;
        fscanf(fd, "%d", &size);
        key_t key = ftok("stack.h", 0);
        assert(key > 0);
        stack = attach_stack(key, size);
        if (stack != NULL)
            printf("stack attached\n\n");
        else
        {
            printf("stack cannot be attached\n");
            return -1;
        }
        int flag = 0;
        while(1)
        {
            flag = handler(fd);
            
            if (flag)
            {
                printf("test finished\n");
                return 0;
            }
        }
    }
    else //working with stdin
    {
        int size;
        printf("Please enter the size of stack\n");
        scanf("%d", &size);
        int flag = 0;
        key_t key = ftok("stack.h", 0);
        assert(key > 0);
        stack = attach_stack(key, size);
        if (stack != NULL)
            printf("stack attached\n\n");
        else
        {
            printf("stack cannot be attached\n");
            return -1;
        }
        print_all(stack);
        printf("This stack includes following instruction:\n\tget_size (get_s)\n\tget_count (get_c)\n\tpush [input] (pu [input])\n\tpop (po)\n\tset_wait (s) [val] [seconds] [nanoseconds] (if val == 1, two numbers must be inputted)\n\tprint_all (pr)\n\tdetach_stack (d)\n\tmark_destruct (m)\n\tfinish (f) (this instruction finishes the test)\n\nPrint any instruction to continue...\n");
        while(1)
        {
            flag = handler(stdin);
            if (flag)
            {
                printf("test finished\n");
                return 0;
            }
        }
    }
    return 0;
}