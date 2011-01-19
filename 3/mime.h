/*
 * mime.h
 *
 * Copyright (c) WEI Zhicheng <zhicheng1988@gmail.com>
 *
 */

#ifndef _MIME_H_
#define _MIME_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mime_part {
	struct mime_part *first_child;
	struct mime_part *next_sibling;

	int multipart;
	int header_offset;
	int content_offset;
	int content_lines;
};


#endif /* _MIME_H_ */
