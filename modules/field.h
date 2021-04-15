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

#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })

#define list_for_each_entry(pos, head, member) \
for (pos = container_of((head)->next, typeof(*(pos)), member);\
&(pos->member)!=(head); pos = container_of(pos->member.next, typeof(*pos), member))

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


extern int sum(int m, int a, int b);
extern int register_field(struct field* f);