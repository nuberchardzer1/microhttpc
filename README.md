# microhttpc

A **minimal and educational HTTP web framework** written in pure C using POSIX sockets.

It provides a simple foundation for building web servers and handling routing, from managing raw TCP connections to parsing HTTP requests and generating responses.

Inspired by the simplicity of Go’s [net/http](https://pkg.go.dev/net/http) package.

--- 
## Example

```C
#include "http.h"

void BaseHandler(response *resp, request *req) {
    resp->status_code = HTTP_STATUS_OK;
    strcpy(resp->status_text, "OK");
    strcpy(resp->content_type, "text/html");

    if (strcmp(req->uri, "/") == 0) {
        int fd = open("index.html", O_RDONLY);
        ssize_t n = read_all(fd, resp->body, sizeof(resp->body));
        close(fd);

        resp->content_length = n;

        strcpy(resp->headers[0].key, "Content-Type");
        strcpy(resp->headers[0].val, "text/html");

        strcpy(resp->headers[1].key, "Content-Length");
        snprintf(resp->headers[1].val, sizeof(resp->headers[1].val), "%zd", n);

        resp->headers_cnt = 2;
    }
}

int main(){
    listen_and_serve("localhost:8080", *BaseHandler);
}
```

---

## Goals

- [x] Build a basic TCP server from scratch
- [x] Handle multiple clients using `fork()`
- [x] Implement HTTP/1.1 request parsing
- [ ] Implement persistent connections (Keep-Alive)
- [ ] Add MIME type detection
- [x] Implement HTTP response helpers
```
void send_404(response *r);
void send_500(response *r);
void send_file(response *r, const char *path);
```
- [ ] Improve headers convenience 
- [ ] Add routing system
- [ ] Add logging and configuration
- [ ] Graceful shutdown (SIGINT)
- [ ] Explore multithreading with `pthread`
