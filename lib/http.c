#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include "http.h"
#include "header.h"
#include "rio.h"

#define CRLF "\r\n"

int read_payload(void *body, rio_t *rio, int n);

int read_request(rio_t *rio, request *req){
    int i, n;
    char line[HTTP_MAX_LINE_LEN], method[HTTP_MAX_LINE_LEN], uri[HTTP_MAX_LINE_LEN];

    n = rio_readline(rio, line, HTTP_MAX_LINE_LEN);
    if (n <= 0){
        return n;
    }
    // First line: GET /index.html HTTP/1.0
    sscanf(line, "%s %s %s", req->method, req->uri, req->proto);
    if (strlen(req->method) < 0 || strlen(req->uri) < 0 || strlen(req->proto) < 0)
        return -1;

    if (read_headers(rio, req->headers, HTTP_MAX_HEADERS)){
        return -1;
    }

    i = get_header_pos("Content-Length", req->headers, HTTP_MAX_HEADERS);
    if (i != -1){
        size_t content_length = atoi(req->headers[i].val);

        n = rio_readn(rio, req->body, content_length);
        if (n < 0){
            return -1;
        }
    }

    return 0;
}

void write_status_code(response w, int code){
    if (code < 100 || code > 999){
        printf("wrong status code: %d", code);
        exit(-1);
    }
}

int marshall_response(response *resp, char *usrbuf, size_t size) {
    int n = 0;

    n += snprintf(usrbuf + n, size - n,
                  "%s %d %s" CRLF,
                  resp->req->proto,
                  resp->status_code,
                  resp->status_text);

    header_t *hp = resp->headers;
    for (int i = 0; i < resp->headers_cnt; i++, hp++) {
        n += snprintf(usrbuf + n, size - n, "%s: %s" CRLF, hp->key, hp->val);
    }

    n += snprintf(usrbuf + n, size - n, CRLF);

    if (resp->content_length > 0) {
        memcpy(usrbuf + n, resp->body, resp->content_length);
        n += resp->content_length;
    }

    return n;
}
