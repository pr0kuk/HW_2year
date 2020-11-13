#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char* argv[], char* envp[])
{
    int i = 0;
    char * env   = (char*)malloc(100);
    char * val   = (char*)malloc(100);
    char * targv = (char*)malloc(200);
    for (i = 2; i < argc; i++)
    {
        targv = argv[i];
        env = strtok_r(targv, "=", &targv);
        val = strtok_r(targv, "=", &targv);
        setenv(env, val, 0);
    }
    execl("print", "print", NULL);
}