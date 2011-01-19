/*
 * list.c 
 *
 * Copyright (c) WEI Zhicheng <zhicheng1988@gmail.com>
 *
 */

#include "list.h"

void *
tmalloc(size_t size)
{
	void *ret;
	if ((ret = malloc(size)) == NULL) {
		fprintf(stderr, "out of mem\n");
		exit(1);
	}

	return ret;
}

void *
trealloc(void *mem, size_t size)
{
	void *ret;
	if ((ret = realloc(mem, size)) == NULL) {
		fprintf(stderr, "out of mem\n");
		exit(1);
	}
	return ret;
}

list *
list_create(type t)
{
	list *l = tmalloc(sizeof(list));
	l->count = 0;
	l->mapn  = 100;
	l->type = t;
	if (t == type_str)
		l->map   = tmalloc(sizeof(char *) * 100);
	else 
		l->map   = tmalloc(sizeof(int *) * 100);
	return l;
}

void
list_append(list *l, void *data)
{
	if (l->count >= l->mapn) {
		l->map = trealloc(l->map, (l->count + 100) * sizeof(void *));
		l->mapn += 100;
	}
	if (l->type == type_str) {
		l->map[l->count] = strdup(data);
	} else {
		int *v = tmalloc(sizeof(int));
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
		int *v = tmalloc(sizeof(int));
		memcpy(v, data, sizeof(int));
		l->map[index] = v;
	}
}

int
list_index(list *l, void *data)
{
	int i;

	if (l->type == type_str) {
		for (i = 0; i < l->count; i++) {
			if (strcmp(data, list_get(l, i)) == 0) {
				return i;
			}
		}

	} else if (l->type == type_int) {
		for (i = 0; i < l->count; i++) {
			if (*(int *)list_get(l, i) == *(int *)data) {
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

