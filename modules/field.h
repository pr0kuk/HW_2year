#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#define STEPS 100000000
struct field {
    int m;
    int (*sum)(int a, int b);
};
extern int sum(int m, int a, int b);
extern int register_field(struct field* f);