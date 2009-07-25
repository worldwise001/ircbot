#ifndef LLIST_H_
#define LLIST_H_

#include <stdlib.h>

typedef struct {
	void * item;
	void * next;
} llist_t;

llist_t * append_item(llist_t * first, void * item);
llist_t * insert_item(llist_t * first, void * item, int location);
llist_t * delete_item(llist_t * first, int location);

void clear_list(llist_t * first);

llist_t * get_item(llist_t * first, int location);

int list_size(llist_t * first);

#endif //LLIST_H_

