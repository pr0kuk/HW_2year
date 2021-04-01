#include "field.h"
static struct field* loaded = NULL;
static int m = 16;
int main(int argc, char * argv[])
{
    int a = 12345678, b = 738654231, s = 0;
    if (dlopen(argv[1], RTLD_NOW) == NULL)
        printf("ERRPR\n");
    for (int i = 0; i < STEPS; i++) {
        s += sum(m, a,b);
    }
    printf("%d\n", s);
    return 0;
}