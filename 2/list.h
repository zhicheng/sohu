
#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *
tmalloc(size_t size);

void *
trealloc(void *mem, size_t size);

typedef enum _type type;
enum _type {type_str, type_int};

typedef struct _list list;
struct _list {
	type     type;
	size_t   count;
	size_t   mapn;
	void   **map;
};

list *
list_create(type type);

void
list_append(list *l, void *data);

size_t 
list_count(list *l);

void *
list_get(list *l, int index);

void
list_set(list *l, void *data, int index);

int
list_index(list *l, void *data);

void
list_destroy(list *l);

#endif /* _LIST_H_ */
