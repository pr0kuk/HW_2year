#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
int main(int argc, char* argv[], char* envp[])
{
    int i = 0;
    //char** substr = (char**)malloc(2*50);
    char * env = (char*)malloc(100);
    char * val = (char*)malloc(100);
    char** com = (char**)malloc(2*strlen(argv[1]));
    char* targv = (char*)malloc(200);
    for (i = 0; i < argc; i++)
    {
        printf("argv[%d] is %s\n", i, argv[i]);
    }
    printf("argc is %d\n", argc);
    for (i = 2; i < argc; i++)
    {
        //printf("j is %d\n", j);
        targv = argv[i];
        //printf("targv[%d] is %s\n", j, targv);
        env = strtok_r(targv, "=", &targv);
        printf("env %s ", env);
        val = strtok_r(targv, "=", &targv);
        printf("val %s\n", val);
        setenv(env, val, 0);
    }
    /*targv = argv[1];
    for (i = 0; com[i] = strtok_r(targv, " ", &targv); i++)
    {
        printf("%s\n", com[i]);
    }
    for (i = 0; i < 5; i++)
    {
        printf("%s\n", com[i]);
    }
    execvp(com[0], com);*/
    //execvp()
    char* check = getenv("MY_VAR");
    printf("MY_VAR is %s\n",check);
    printf("\n\n");
    for (i = 0; envp[i]; i++)
        printf("%s\n", envp[i]);
}