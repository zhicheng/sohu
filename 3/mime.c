
#include "mime.h"

char *
strndup(char *src, size_t size)
{
	char *dest = (char *)malloc(size + 1);
	memset(dest, 0, size + 1);
	strncpy(dest, src, size);
	return dest;
}

char *
mime_get_header_field(const char *header, const char *field)
{
	char *ptr, *tmp, *ret;
	char *sep = " \t\n\r";
	size_t size;
	
	for (ptr = strstr(header, field); ptr != NULL; ptr = strstr(ptr, field)) {
		if (ptr == header || (*(ptr - 1)) == '\n')
			break;
		ptr += strlen(field);
	}

	ptr += strlen(field);
	if (*ptr == ':')
		ptr += 1;

	// remove space
	while (*ptr == ' ')
		ptr++;

	// find field end
	for (tmp = ptr; *tmp != '\0'; tmp++) {
		if (*tmp == '\n') {
			if ((*(tmp + 1)) != '\t' 
			 && (*(tmp + 1)) != ' ')
				break;
		}
	}

	size = tmp - ptr;

	ret = malloc(size + 1);
	memset(ret, 0, size + 1);

	tmp = strndup(ptr, size);

	// remove '\t', ' ', '\n', '\r'
	for (tmp = strtok(tmp, sep); tmp != NULL; tmp = strtok(NULL, sep))
		strcat(ret, tmp);

	return ret;
}


int
main(int argc, char *argv[])
{
	FILE *fp;
	char *buf;
	int filesize;
	int n;

	char *header;
	char *ptr;

	fp = fopen(argv[1], "r");

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf = (char *)malloc(filesize + 1);
	n = fread(buf, 1, filesize, fp);
	buf[n] = 0;

	ptr = strstr(buf, "\r\n\r\n");

	header = strndup(buf, ptr - buf);
	printf("%s\n", mime_get_header_field(header, "To"));
	printf("%s\n", mime_get_header_field(header, "Content-Type"));

	return 0;
}

