
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "list.h"

#define TITLES_SIZE   8

static list *titles = NULL;
static list *keys   = NULL;
static list *title_keys[TITLES_SIZE];

size_t 
file_size(FILE *fp)
{
	size_t ret;
	off_t  off;
	off = ftell(fp);
	fseek(fp, 0, SEEK_END);
	ret = (size_t)ftell(fp);
	fseek(fp, off, SEEK_SET);

	return ret;
}

char *
strlower(char *s)
{
	char *str = s;
	while (*str) {
		*str = tolower(*str);
		str++;
	}
	return s;
}

list *
separatewords(char *text)
{
	list *l;
	char *word;
	char *sep = " -,.?:/@()<>[]{}\'\"\t\r\n";

	l = list_create(type_str);

	for (word = strtok(text, sep); 
	     word != NULL; word = strtok(NULL, sep)) {
		if (strlen(word) <= 3)
			continue;
		list_append(l, strlower(word));
	}
	return l;
}

list *
get_dirs(char *dirpath)
{
	DIR *dp;
	struct dirent *dirp;

	list *l;
	l = list_create(type_str);

	if ((dp = opendir(dirpath)) == NULL) {
		fprintf(stderr, "can't open %s directory\n", dirpath);
		return NULL;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if (strncmp(dirp->d_name, ".", 1) == 0)
			continue;
		list_append(l, dirp->d_name);
	}

	if (closedir(dp) < 0) {
		fprintf(stderr, "can't close %s directory\n", dirpath);
		return NULL;
	}

	return l;
}

int 
title_count()
{
	int ret;
	list *l;
	l = get_dirs("articles");
	ret = list_count(l);
	list_destroy(l);
	return ret;
}

int
file_count()
{
	int i, n = 0;
	list *dirs;
	list *files;
	char *path;
	char *dir;

	dirs = get_dirs("articles");
	for (i = 0; i < list_count(dirs); i++) {
		dir = (char *)list_get(dirs, i);
		path = malloc(strlen(dir) + strlen("articles") + 2);
		sprintf(path, "%s/%s", "articles", dir);
		files = get_dirs(path);
		n += list_count(files);
		list_destroy(files);
		free(path);
	}
	list_destroy(dirs);
	return n;
}

int
file_classification_count(char *title)
{
	int ret;
	list *l;
	char *path;
	path = malloc(strlen(title) + strlen("articles") + 2);
	sprintf(path, "%s/%s", "articles", title);
	l = get_dirs(path);
	ret = list_count(l);
	list_destroy(l);
	free(path);
	return ret;
}

int
title_key_get(char *title, char *key)
{
	int title_index = list_index(titles, title);
	int key_index   = list_index(keys, key);
	if (title_index == -1 || key_index == -1) 
		return 0;
	return *(int *)list_get(title_keys[title_index], key_index);
}

void
title_key_set(char *title, char *key)
{
	int i;
	int title_index = list_index(titles, title);
	int key_index   = list_index(keys, key);
	int v = 0;
	if (key_index == -1) {
		list_append(keys, key);
		for (i = 0; i < TITLES_SIZE; i++) {
			list_append(title_keys[i], &v);
		}
		key_index = 0;
	}
	v = *(int *)list_get(title_keys[title_index], key_index);
	v += 1;
	list_set(title_keys[title_index], &v, key_index);
}

int
training(char *dirpath)
{
	int i, j, k;
	int readn;
	char *buf;
	FILE *fp;

	list *dirs;
	list *files;
	list *words;
	char *path;
	char *dir;
	char *file;
	char *filepath;

	dirs = get_dirs("articles");
	for (i = 0; i < list_count(dirs); i++) {
		dir = (char *)list_get(dirs, i);
		printf("title: %s\n", dir);
		if (strcmp(dir, "alt.bible.prophecy"))
			continue;
		path = malloc(strlen(dir) + strlen("articles") + 2);
		sprintf(path, "%s/%s", "articles", dir);
		files = get_dirs(path);
		for (j = 0; j < list_count(files); j++) {
			file = (char *)list_get(files, j);
			filepath = malloc(strlen(file) + strlen(dir) + strlen("articles") + 2);
			sprintf(filepath, "%s/%s/%s", "articles", dir, file);
			printf(".");
			fp = fopen(filepath, "r");
			buf = malloc(file_size(fp) + 1);
			readn = fread(buf, 1, file_size(fp), fp);
			buf[readn] = 0;
			fclose(fp);

			words = separatewords(buf);

			free(buf);

			for (k = 0; k < list_count(words); k++) {
				title_key_set(dir, list_get(words, k));
			}

			free(filepath);
		}
		printf("\n");
		list_destroy(files);
		free(path);
	}
	list_destroy(dirs);
}

int
key_classification_count(char *dirpath, char *key)
{
	return title_key_get(dirpath, key);
}

float
pc(char *title)
{
	float Nc = file_classification_count(title);
	float N  = file_count();
	return Nc/N;
}

float
pxc(char *title, char *key)
{
	float M   = 0.0f;
	float Nxc = key_classification_count(title, key);
	float Nc  = file_classification_count(title);
	float V   = title_count();
	return (Nxc + 1) / (Nc + M + V);
}

float 
BayesClassifier(list *words, char *title)
{
	float zoomFactor = 10.0f;
	float ret = 1.0f;

	char *word;
	int i;

	for (i = 0; i < list_count(words); i++) {
		word = list_get(words, i);
		ret *= pxc(title, word) * zoomFactor;
	}
	return ret *= pc(title);
}

void
classify(char *file, char *title)
{
	int i;
	int readn;
	FILE *fp;

	char *buf;
	list *dirs;
	list *words;

	if ((fp = fopen(file, "r")) == NULL) {
		fprintf(stderr, "can't open %s file\n", file);
		return;
	}
	buf = malloc(file_size(fp) + 1);
	readn = fread(buf, 1, file_size(fp), fp);
	buf[readn] = 0;
	fclose(fp);

	words = separatewords(buf);

	free(buf);
	
	dirs = get_dirs("articles");
	char *dir;

	int maxn = 2;
	float temp;
	float max = 0.0f;

	for (i = 0; i < list_count(dirs); i++) {
		dir = list_get(dirs, i);
		//printf("%-30s ", dir);
		//printf(" %f\n", BayesClassifier(words, dir));
		if (((temp = BayesClassifier(words, dir)) - max) > 0.000001) {
			max = temp;
			maxn = i;
		}  
	}
	printf("%d\n", !strcmp(title, (char *)list_get(dirs, maxn)));
	list_destroy(dirs);
	list_destroy(words);
}

int 
main(void)
{
	int i, j;
	list *dirs;
	list *files;
	char *path;
	char *dir;
	char *file;
	char *filepath;

	titles = list_create(type_str);
	keys   = list_create(type_str);

	for (i = 0; i < TITLES_SIZE; i++)
		title_keys[i] = list_create(type_int);

	dirs = get_dirs("articles");
	for (i = 0; i < list_count(dirs); i++) {
		dir = (char *)list_get(dirs, i);
		list_append(titles, dir);
	}

	training("articles");

	dirs = get_dirs("articles");
	for (i = 0; i < list_count(dirs); i++) {
		dir = (char *)list_get(dirs, i);
		path = malloc(strlen(dir) + strlen("articles") + 2);
		sprintf(path, "%s/%s", "articles", dir);
		printf("dir: %s\n", dir);
		files = get_dirs(path);
		for (j = 0; j < list_count(files); j++) {
			file = (char *)list_get(files, j);
			filepath = malloc(strlen(file) + strlen(dir) + strlen("articles") + 2);
			sprintf(filepath, "%s/%s/%s", "articles", dir, file);
			//printf("%s\n", filepath);
			classify(filepath, dir);
			free(filepath);

		}
		return 0;
		list_destroy(files);
		free(path);
	}
	list_destroy(dirs);

	return 0;
}
