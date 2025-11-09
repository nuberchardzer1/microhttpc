#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include "http.h"
#include "header.h"
#include "rio.h"

#define CRLF "\r\n"

int read_request_with_timeout(rio_t *rio, request *req, int timeout_msecs){
    int n;
    char line[HTTP_MAX_LINE_LEN];
    
    n = rio_readline_with_timeout(rio, line, HTTP_MAX_LINE_LEN, timeout_msecs);
    if (n <= 0){
        return -1;
    }
    // First line: GET /index.html HTTP/1.0
    sscanf(line, "%s %s %s", req->method, req->uri, req->proto);
    if (req->method[0] == '\0' || req->uri[0] == '\0' || req->proto[0] == '\0') {
        return -1;
    }
    if (read_headers(rio, &req->headers, HTTP_MAX_HEADERS) < 0){
        return -1;
    }
    char *c = get_header(&req->headers, "Content-Length");
    if (c != NULL){
        size_t content_length = atoi(c);
        req->content_length = content_length;
        n = rio_readn(rio, req->body, content_length);
        if (n < 0){
            return -1;
        }
    }

    return 0;
}

int marshall_response(response *resp, char *usrbuf, size_t size) {
    int n = 0;

    n += snprintf(usrbuf + n, size - n,
                  "%s %d %s" CRLF,
                  resp->req->proto,
                  resp->status_code,
                  resp->status_text);

    for (int i = 0; i < HEADER_BUCKETS; i++) {
        header_t *hp = resp->headers.buckets[i];
        while (hp) {
            n += snprintf(usrbuf + n, size - n, "%s: %s" CRLF, hp->key, hp->val);
            hp = hp->next;
            }
        }

    n += snprintf(usrbuf + n, size - n, CRLF);

    if (resp->content_length > 0) {
        memcpy(usrbuf + n, resp->body, resp->content_length);
        n += resp->content_length;
    }

    return n;
}

int send_http_error(void *usrbuf, int code, const char *reason, const char *body) {
    char *buf = usrbuf;
    size_t body_len = strlen(body);

    int n = snprintf(buf, MAXPAYLOAD,
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        code, reason, body_len, body);

    return n;
}

void send_404(response *resp, request *req){
    resp->status_code = HTTP_STATUS_NOT_FOUND;

    strncpy(resp->body, NOT_FOUND, sizeof(resp->body) - 1);

    add_header(&resp->headers, "Content-Type", "text/plain");
    char len_buf[32];
    sprintf(len_buf, "%zu", strlen(NOT_FOUND));
    add_header(&resp->headers, "Content-Length", len_buf);
    add_header(&resp->headers, "Connection", "close");
    resp->content_length = strlen(NOT_FOUND);
}

void send_500(response *resp) {
    resp->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;

    strncpy(resp->body, INTERNAL_ERROR, sizeof(resp->body) - 1);

    size_t err_len = strlen(INTERNAL_ERROR);

    add_header(&resp->headers, "Content-Type", "text/plain");
    char len_buf[32];
    sprintf(len_buf, "%zu", err_len);
    add_header(&resp->headers, "Content-Length", len_buf);
    add_header(&resp->headers, "Connection", "close");
    resp->content_length = err_len;
}

void redirect(response *resp, int code, char *location) {
    resp->status_code = code;

    add_header(&resp->headers, "Location", location);
    char len_buf[32];
    sprintf(len_buf, "%zu", strlen(NOT_FOUND));
    add_header(&resp->headers, "Content-Length", len_buf);
    add_header(&resp->headers, "Connection", "close");

    resp->content_length = 0;
}

void send_file(response *resp, const char *path){
    int fd = open(path, O_RDONLY);

    resp->status_code = HTTP_STATUS_OK;
    strcpy(resp->status_text, "OK");

    ssize_t n = read_all(fd, resp->body, sizeof(resp->body));
    close(fd);

    add_header(&resp->headers, "Content-Type", "text/html");
    char len_buf[32];
    sprintf(len_buf, "%zd", n);
    add_header(&resp->headers, "Content-Length", len_buf);
    
    resp->content_length = n;
}