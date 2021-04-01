#include "field.h"

static int sum16(int a, int b)
{
    return (a+b) & 0b1111;
}

static struct field m16 = {
    .m = 16,
    .sum = sum16
};



__attribute__((constructor))static void my_load() {
    register_field(&m16);
}