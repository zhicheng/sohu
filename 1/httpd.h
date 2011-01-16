#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>

#include <event.h>
#include <evhttp.h>
#include <evdns.h>
#include <sys/queue.h>

#define SERVER_SIGNATURE "libevent server v 0.0.1"
#define UPLOAD_URI   "/upload.php"
#define PROGRESS_URI "/progress"

struct _options {
	int   port;
	char *address;
	int   verbose;
} options;

void 
files(const char *uri, struct evhttp_request *req);

void 
upload(struct evhttp_request *req, void *arg);

void 
progress(struct evhttp_request *req, void *arg);
