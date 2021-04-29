#include "my_server.h"
#include <math.h>
//static long double g = 23, p = 256;
//static long double a = 4, b = 11;
char gen_open_key_server(long double g, long double a, long double p) {
    long double t = powl(g,a);
    return (char)fmodl(t, p);
}

char gen_open_key_client(long double g, long double b, long double p) {
    long double t = powl(g,b);
    return (char)fmodl(t, p);
}

char gen_close_key_server(char B, long double a, long double p) {
    long double t = powl(B,a);
    return (char)fmodl(t, p);
}

char gen_close_key_client(char A, long double b, long double p) {
    long double t = powl(A, b);
    return (char)fmodl(t, p);
}