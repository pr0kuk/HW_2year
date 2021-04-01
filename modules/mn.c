#include "field.h"
static int sumn(int a, int b);


static struct field mn = {
    .m = 0,
    .sum = sumn
};

static int sumn(int a, int b)
{
    return (a+b) % mn.m;
}

__attribute__((constructor))static void my_load() {
    register_field(&mn);
}