#include "field.h"



#include <stdio.h>





static inline void __list_add(struct list* item, struct list* prev, struct list* next)
{
    prev->next = item;
    next->prev = item;
    item->prev = prev;
    item->next = next;
}

void list_init(struct list * list)
{
    list->prev = list;
    list->next = list;
}

void list_add(struct list* new, struct list* head)
{
    __list_add(new, head, head->next);
}
void list_add_back(struct list* new, struct list* head) //?
{
    __list_add(new, head->prev, head);
}

static void list_del(struct list* old)
{
    old->prev->next = old->next;
    old->next->prev = old->prev;
}

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