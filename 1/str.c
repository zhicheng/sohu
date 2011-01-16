#include <str.h>

char *index_str(const char *haystack, size_t hl, const char *needle)
{
	size_t nl = strlen(needle);
	size_t i;
	if (!nl) goto found;
	if (nl > hl) return 0;

	for (i = hl - nl + 1; i > 0; --i) {
		if (*haystack == *needle && !memcmp(haystack, needle, nl))
found:
    			return (char *)haystack;
		++haystack;
	}
	return 0;
}

