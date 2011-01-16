
#include "list.h"

list *
list_create(type t)
{
	list *l = malloc(sizeof(list));
	l->count = 0;
	l->mapn  = 100;
	l->type = t;
	if (t == type_str)
		l->map   = malloc(sizeof(char *) * 100);
	else 
		l->map   = malloc(sizeof(int *) * 100);
	return l;
}

void
list_append(list *l, void *data)
{
	if (l->count >= l->mapn) {
		l->map = realloc(l->map, (l->count + 100) * sizeof(void *));
		l->mapn += 100;
	}
	if (l->type == type_str) {
		l->map[l->count] = strdup(data);
	} else {
		int *v = malloc(sizeof(int));
		memcpy(v, data, sizeof(int));
		l->map[l->count] = v;
	}
	l->count += 1;
}

size_t 
list_count(list *l)
{
	return l->count;
}

void *
list_get(list *l, int index)
{
	return l->map[index];
}

void
list_set(list *l, void *data, int index)
{
	if (index > l->count) {
		return;
	}
	if (data != l->map[index]) {
		free(l->map[index]);
	}
	if (l->type == type_str) {
		l->map[index] = strdup(data);
	} else {
		int *v = malloc(sizeof(int));
		memcpy(v, data, sizeof(int));
		l->map[index] = v;
	}
}

int
list_index(list *l, void *data)
{
	int i;
	for (i = 0; i < l->count; i++) {
		if (l->type == type_str) {
			if (strcmp(data, list_get(l, i)) == 0) {
				return i;
			}
		} else {
			if (*(int *)data == *(int *)list_get(l, i)) {
				return i;
			}

		}
	}
	return -1;
}

void
list_destroy(list *l)
{
	int i;
	for (i = 0; i < l->count; i++) {
		free(l->map[i]);
	}
	free(l->map);
	free(l);
}

