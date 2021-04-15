#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stddef.h>
#define STEPS 100000000


struct list
{
    struct list *next;
    struct list *prev;
};


struct field {
    int m;
    int (*sum)(int a, int b);
    struct list list;
};

#define LIST_HEAD(name) struct list name = {&(name), &(name)}


#define container_of(ptr, type, member) \
({const typeof(((type*)0)->member) *__mptr = (ptr);\
(type*)(char*)__mptr - offsetof(type,member);})

#define list_for_each_entry(pos, head, member) \
for (pos = container_of((head)->next, typeof(*(pos)), member);\
&(pos->member)!=(head); pos = container_of(pos->member.next, typeof(*pos), member))




void list_add(struct list* new, struct list* head);
extern int sum(int m, int a, int b);
extern int register_field(struct field* f);