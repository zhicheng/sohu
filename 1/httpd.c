#include "httpd.h"
#include "str.h"

#define BUFFER_SIZE  1024
#define MULTI_TYPE   "multipart/form-data"

void parse_mime(const unsigned char *data, size_t data_len, const char *boundary,
    unsigned char *dest[], size_t dest_size);

void 
upload_html(struct evhttp_request *req, void *arg)
{
	int n;
	FILE *fp;
	struct evbuffer *evbuff;
	unsigned char *buffer = malloc(BUFFER_SIZE);

	if (req->type != EVHTTP_REQ_GET) {
		evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
		return;
	}

	evbuff = evbuffer_new();
	fp = fopen("upload.html", "r");
	while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
		buffer[n] = 0;
		evbuffer_add_printf(evbuff, "%s", buffer);
	}
	fclose(fp);

	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=UTF-8");
	evhttp_add_header(req->output_headers, "Server", SERVER_SIGNATURE);

	evhttp_send_reply(req, HTTP_OK, "", evbuff);
	evbuffer_free(evbuff);
}

void 
upload_php(struct evhttp_request *req, void *arg)
{
	if (req->type != EVHTTP_REQ_POST) {
		evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
		return;
	}
	const char *content_type = evhttp_find_header(req->input_headers,
	    "Content-Type");
	const char *content_length = evhttp_find_header(req->input_headers,
	    "Content-Length");
	int length = atoi(content_length);
	int footer_length = 0;
	printf("Content-Type: %s\n", content_type);
	printf("Content-Length: %d\n", length);

	if (strlen(content_type) < 30) {
		evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
		return;
	}

	if (strncmp(content_type, MULTI_TYPE, strlen(MULTI_TYPE)) != 0) {
		evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
		return;
	}

	unsigned char *data = EVBUFFER_DATA(req->input_buffer);
	size_t len = EVBUFFER_LENGTH(req->input_buffer);

	printf("boundary: %s\n", content_type + 30);

	//printf("data: %s\n", data);
	char *sep = malloc(strlen(content_type + 30) + 4);
	sprintf(sep, "--%s\r\n", content_type + 30);
	parse_mime(data, len, content_type + 30, NULL, 0);
	
	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=UTF-8");
	evhttp_add_header(req->output_headers, "Server", SERVER_SIGNATURE);
	evhttp_send_reply(req, HTTP_OK, "", NULL);
}

int next_part(const unsigned char *data, size_t data_len, const char *boundary,
    char **part_ptr, size_t *part_len)
{
	char *sep = malloc(strlen(boundary) + 4);
	sprintf(sep, "--%s\r\n", boundary);

	size_t len;
	char *left;
	char *right;
	left = index_str(data, data_len, boundary);
	right = index_str(data + strlen(boundary), data_len - strlen(boundary),
	    boundary);
	if (left == 0 || right == 0)
		return -1;
	len = right - left - strlen(boundary);
	char *part = malloc(len);
	memcpy(part, left + strlen(boundary), len);
	*part_ptr = part;
	*part_len = len;
	return 0;
}

void parse_mime(const unsigned char *data, size_t data_len, const char *boundary,
    unsigned char *dest[], size_t dest_size)
{
	char *part;
	size_t *len;
	if (next_part(data, data_len, boundary, &part, len))
		printf("part: %s\n", part);
}

void 
progress(struct evhttp_request *req, void *arg)
{
	if (req->type != EVHTTP_REQ_GET) {
		evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
		return;
	}
	printf("progress\n");
	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=UTF-8");
	evhttp_add_header(req->output_headers, "Server", SERVER_SIGNATURE);
	evhttp_send_reply(req, HTTP_OK, "", NULL);
}

int main(int argc, char **argv)
{
	int opt;

	options.port = 8080;
	options.address = "0.0.0.0";
	options.verbose = 0;

	while ((opt = getopt(argc, argv, "p:vh")) != -1) {
		switch (opt) {
			case 'p':
				options.port = atoi(optarg);
				break;
			case 'v':
				options.verbose = 1;
				break;
			case 'h':
				printf("Usage: libsrvr -p port -v[erbose] -h[help]\n");
				exit(1);
		}
	}

	struct event_base *evbase = NULL;
	struct evhttp *evhttp = NULL;

	evbase = event_init();
	evhttp = evhttp_new(evbase);

	evhttp_set_cb(evhttp, "/upload.html", upload_html, NULL);
	evhttp_set_cb(evhttp, "/upload.php", upload_php, NULL);
	evhttp_set_cb(evhttp, "/progress", progress, NULL);

	evhttp_bind_socket(evhttp, "0.0.0.0", options.port);
	event_base_dispatch(evbase);

	evhttp_free(evhttp);

	return 0;
}
