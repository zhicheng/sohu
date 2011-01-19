/*
 * main.c
 *
 * Copyright (c) WEI Zhicheng <zhicheng1988@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "list.h"

#define ARTICLES_NUM 		8
#define ARTICLES_FOLDER "articles"

static list *articles = NULL;
static list *words   = NULL;
static list *article_words[ARTICLES_NUM];
static int   article_files_num[ARTICLES_NUM];

FILE *tmpfp;

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
	char *sep = " -_,.?:/@()<>[]{}\'\"\t\r\n";

	l = list_create(type_str);

	for (word = strtok(text, sep); 
	     word != NULL; 
	     word = strtok(NULL, sep)) {
		if (strlen(word) <= 3)
			continue;
		list_append(l, strlower(word));
	}
	return l;
}

list *
dir_ls(char *dirpath)
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

list *
get_words(char *file)
{
	FILE   *fp;
	char   *buf;
	list   *words;
	size_t  readn;

	if ((fp = fopen(file, "r")) == NULL) {
		fprintf(stderr, "can't open %s file\n", file);
		exit(1);
	}

	buf = tmalloc(file_size(fp) + 1);
	readn = fread(buf, 1, file_size(fp), fp);
	buf[readn] = 0;
	fclose(fp);

	words = separatewords(buf);

	free(buf);

	return words;
}

int 
article_count()
{
	return ARTICLES_NUM;
}

int
file_count()
{
	int i;
	int ret = 0;
	for (i = 0; i < article_count(); i++) {
		ret += article_files_num[i];
	}
	return ret;
}

int
article_file_count(char *article)
{

	int article_index = list_index(articles, article);
	if (article_index != -1) {
		return article_files_num[article_index];
	} 
	return 0;
}

int
article_word_get(char *article, char *word)
{
	int article_index = list_index(articles, article);
	int word_index    = list_index(words, word);
	if (article_index == -1 || word_index == -1)  {
		return 0;
	}
	return *(int *)list_get(article_words[article_index], word_index);
}

void
article_word_set(char *article, char *word)
{
	int i;
	int article_index = list_index(articles, article);
	int word_index    = list_index(words, word);
	int v = 0;
	if (word_index == -1) {
		list_append(words, word);
		for (i = 0; i < article_count(); i++) {
			list_append(article_words[i], &v);
		}
		word_index = list_index(words, word);
	}
	v = *(int *)list_get(article_words[article_index], word_index);
	v += 1;
	list_set(article_words[article_index], &v, word_index);
}

void
tranining(char *article, char *file)
{
	int i;
	list *words;
	words = get_words(file);
	for (i = 0; i < list_count(words); i++) {
		article_word_set(article, list_get(words, i));
	}
	list_destroy(words);
}


int
article_word_count(char *article, char *word)
{
	return article_word_get(article, word);
}

float
pc(char *article)
{
	float Nc = article_file_count(article);
	float N  = file_count();
	return Nc/N;
}

float
pxc(char *article, char *word)
{
	float M   = 0.0f;
	float Nxc = article_word_count(article, word);
	float Nc  = article_file_count(article);
	float V   = article_count();
	return (Nxc + 1) / (Nc + M + V);
}

float 
BayesClassifier(list *words, char *article)
{
	float zoomFactor = 40.0f;
	float ret = 1.0f;

	char *word;
	int i;

	for (i = 0; i < list_count(words); i++) {
		word = list_get(words, i);
		ret *= pxc(article, word) * zoomFactor;
	}
	return ret *= pc(article);
}

void
classify(char *file, char *article)
{
	int i;

	list *dirs;
	list *words;
	
	float  max  = 0.0f;
	int    maxn = -1;
	float  temp;
	char  *dir;

	dirs = dir_ls(ARTICLES_FOLDER);
	words = get_words(file);

	for (i = 0; i < list_count(dirs); i++) {
		dir = list_get(dirs, i);
		if (((temp = BayesClassifier(words, dir)) - max) > 0.00000001) {
			max = temp;
			maxn = i;
		}  
	}
	printf("article: %s\n", (char *)list_get(dirs, maxn));
	printf("result: ");
	if (maxn == -1) {
		printf("wrong\n\n");
	}
	else
		printf("%s\n\n", strcmp(article, list_get(dirs, maxn)) ?
		    "wrong" : "right");
	list_destroy(dirs);
	list_destroy(words);
}

void
trainingall(char *dirpath)
{
	int i, j;

	list *dirs;
	list *files;
	char *path;
	char *dir;
	char *file;
	char *filepath;

	int percent = 90;   // 90%

	srand(((unsigned int)time(NULL)) | 1);

	dirs = dir_ls(ARTICLES_FOLDER);
	for (i = 0; i < list_count(dirs); i++) {
		dir = (char *)list_get(dirs, i);
		printf("training: %s\n", dir);
		path = tmalloc(strlen(dir) + strlen(ARTICLES_FOLDER) + 3);
		sprintf(path, "%s/%s", ARTICLES_FOLDER, dir);
		files = dir_ls(path);

		int article_index = list_index(articles, dir);
		if (article_index == -1) 
			continue;

		for (j = 0; j < list_count(files); j++) {
			file = (char *)list_get(files, j);
			filepath = tmalloc(strlen(file) + strlen(dir) + strlen(ARTICLES_FOLDER) + 3);
			sprintf(filepath, "%s/%s/%s", ARTICLES_FOLDER, dir, file);
			if ((rand() % 100) >= percent) {
				fwrite(filepath, 1, strlen(filepath), tmpfp);
				fwrite("\n", 1, 1, tmpfp);
				goto next;
			}
			tranining(dir, filepath);
			article_files_num[article_index] += 1;
next:
			free(filepath);
		}
		list_destroy(files);
		free(path);
	}
	list_destroy(dirs);
}

int 
main(void)
{
	int i;
	list *dirs;
	char *dir;
	char file[100];

	articles = list_create(type_str);
	words    = list_create(type_str);

	for (i = 0; i < article_count(); i++)
		article_words[i] = list_create(type_int);

	dirs = dir_ls(ARTICLES_FOLDER);
	for (i = 0; i < list_count(dirs); i++) {
		dir = (char *)list_get(dirs, i);
		list_append(articles, dir);
	}
	list_destroy(dirs);

	if ((tmpfp = fopen("temp.txt", "w+")) == NULL) {
		fprintf(stderr, "tmpfile() fail\n");
		exit(1);
	}

	trainingall(ARTICLES_FOLDER);

	printf("training end\n\n");

	dirs = dir_ls(ARTICLES_FOLDER);
	fseek(tmpfp, 0, SEEK_SET);
	
	char *sep = "/";
	while (fgets(file, sizeof(file), tmpfp) != NULL) {
		file[strlen(file) - 1] = 0;
		printf("file: %s\n", file);
		char *file2 = strdup(file);
		strtok(file2, sep);
		classify(file, strtok(NULL, sep));
		free(file2);
	}
	list_destroy(dirs);

	return 0;
}
