#include "field.h"
static LIST_HEAD(fields);
int register_field(struct field* f)
{
    //if (f->m > 256)
    //    return -1;
    //fields[f->m] = f;
    list_add(&f->list, &fields);
    return 0;
}

struct field* get_field(int m)
{
    struct field *f;
    list_for_each_entry(f, &fields, list)
    {
        if (f->m == m)
            return f;
    }
    return NULL; //container_of((&fields)->next, &fields, list);
}

int sum(int m, int a, int b)
{
    struct field* f;
    
    /*f = fields[m];
    if (!f) {
        f = fields[0];
        f->m = m;
    }*/
    return f->sum(a,b);
}