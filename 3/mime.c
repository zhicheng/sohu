/*
 * mime.c
 *
 * Copyright (c) WEI Zhicheng <zhicheng1988@gmail.com>
 *
 */

#include "mime.h"

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

char *
strndup(const char *src, size_t size)
{
	char *dest = (char *)tmalloc(size + 1);
	memset(dest, 0, size + 1);
	strncpy(dest, src, size);
	return dest;
}

size_t
line_num(const char *str)
{
	int n = 0;
	for (;*str != '\0'; str++)
		if (*str == '\n')
			n++;
	return n;
}

char *
mime_get_type(const char *content_type)
{
	char *ptr;
	char *type = NULL;
	if ((ptr = strchr(content_type, ';')) != NULL) 
		type = strndup(content_type, ptr - content_type);
	return type;
}

char *
mime_get_boundary(const char *content_type)
{
	char *ptr, *tmp;
	char *boundary = NULL;

	if ((tmp = strstr(content_type, "boundary")) == NULL) 
		return NULL;
	ptr = tmp + strlen("boundary");
	if (*ptr != '=') 
		return NULL;

	// find '"' start
	if ((tmp = strchr(ptr, '"')) == NULL)
		return NULL;
	ptr = tmp + 1;
	// find '"' end
	if ((tmp = strchr(ptr, '"')) == NULL)
		return NULL;

	boundary = strndup(ptr, tmp - ptr);

	return boundary;
}

char *
mime_get_header_field(const char *header, const char *field)
{
	char *ptr, *tmp, *ret;
	char *sep = " \t\n\r";
	size_t size;
	
	// find field start
	for (ptr = strstr(header, field); ptr != NULL; ptr = strstr(ptr, field)) {
		if (ptr == header || (*(ptr - 1)) == '\n')
			break;
		ptr += strlen(field);
	}

	if (ptr == NULL)
		return NULL;

	ptr += strlen(field);
	if (*ptr == ':')
		ptr += 1;

	// remove space
	while (*ptr == ' ')
		ptr++;

	// find field end
	for (tmp = ptr; *tmp != '\0'; tmp++)
		if (*tmp == '\n') 
			if ((*(tmp + 1)) != '\t' 
			 && (*(tmp + 1)) != ' ')
				break;

	size = tmp - ptr;

	ret = tmalloc(size + 1);
	memset(ret, 0, size + 1);

	tmp = ptr = strndup(ptr, size);
	// remove '\t', ' ', '\n', '\r'
	for (tmp = strtok(tmp, sep); tmp != NULL; tmp = strtok(NULL, sep))
		strcat(ret, tmp);
	free(ptr);

	return ret;
}

struct mime_part *
mime_get_part(const char *body, const char *header_boundary)
{
	char *ptr, *end, *data;

	char *type;
	char *boundary;
	char *content_type;
	char *local_boundary;
	struct mime_part *part = NULL;
	struct mime_part *next = NULL;
	struct mime_part *tmp  = NULL;

	size_t blen = strlen(header_boundary);
	size_t partlen = sizeof(struct mime_part);

	boundary = tmalloc(blen + 3);    /* "--" and "\0" */
	sprintf(boundary, "--%s", header_boundary);
	blen += 2;

	for (ptr = strstr(body, boundary); ptr != NULL;) {
		end = strstr(ptr + blen, boundary);
		content_type = mime_get_header_field(ptr, "Content-Type");

		tmp = tmalloc(partlen);
		tmp->next_sibling = NULL;
		tmp->first_child = NULL;
		tmp->multipart = 0;
		if (next != NULL) {
			next = next->next_sibling = tmp;
		} else {
			part = next = tmp;
		}

		next->header_offset = ptr - body + blen + 2; /* "\r\n" */
		ptr = strstr(ptr, "\r\n\r\n");
                ptr += 4;                       /* "\r\n\r\n" */
		next->content_offset = ptr - body;
		data = strndup(ptr, end - ptr - 2);   /* "\r\n" */
		next->content_lines = line_num(data);
		free(data);

		type = mime_get_type(content_type);
		if (!strncmp(type, "multipart", 9)) {
			local_boundary = mime_get_boundary(content_type);
			next->multipart = 1;
			next->first_child = mime_get_part(body, local_boundary);
			free(local_boundary);
		} 
		free(type);

		free(content_type);
		if (!strncmp(end + blen, "--", 2)) 
			break;
		ptr = end;
	}

	free(boundary);
	return part;
}

void
indent(int width)
{
	while (width--)
		putchar('\t');
}

void
travel(struct mime_part *part, int dept)
{
	indent(dept);
	printf("multipart: %d\n", part->multipart);
	indent(dept);
	printf("header_offset: %d\n", part->header_offset);
	indent(dept);
	printf("content_offset: %d\n", part->content_offset);
	indent(dept);
	printf("content_lines: %d\n\n", part->content_lines);

	if (part->first_child != NULL) {
		travel(part->first_child, dept + 1);
	}
	if (part->next_sibling != NULL) {
		travel(part->next_sibling, dept);
	}
}

void
release(struct mime_part *part)
{
	if (part->first_child != NULL) {
		release(part->first_child);
	}
	if (part->next_sibling != NULL) {
		release(part->next_sibling);
	}
	free(part);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	char *buf;
	char *type;
	char *boundary;
	char *content_type;
	char *ptr;

	int n;
	int filesize;

	struct mime_part *part;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(1);
	}

	if ((fp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "can't open %s file\n", argv[1]);
	}

	// get file size
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// read file
	buf = (char *)tmalloc(filesize + 1);
	n = fread(buf, 1, filesize, fp);
	buf[n] = 0;

	content_type = mime_get_header_field(buf, "Content-Type");

	type = mime_get_type(content_type);

	// only support multipart type email
	if (strncmp(type, "multipart", 9)) {
		fprintf(stderr, "can't support\n");
		exit(0);
	}

	free(type);

	if ((ptr = strstr(buf, "\r\n\r\n")) == NULL) {
		fprintf(stderr, "wrong mime file\n");
		exit(1);
	}
	part = tmalloc(sizeof(struct mime_part));
	part->multipart = 1;
	part->next_sibling = NULL;
	part->header_offset = 0;
	part->content_offset = ptr - buf + 4;  /* "\r\n\r\n" */
	part->content_lines = line_num(buf + part->content_offset);

	boundary = mime_get_boundary(content_type);
	free(content_type);

	part->first_child = mime_get_part(buf, boundary);

	free(boundary);
	free(buf);

	travel(part, 0);
	release(part);

	return 0;
}

