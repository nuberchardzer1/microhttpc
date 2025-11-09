# microhttpc

A **minimal and educational HTTP web framework** written in pure C using POSIX sockets.

It provides a simple foundation for building web servers and handling routing, from managing raw TCP connections to parsing HTTP requests and generating responses.

Inspired by the simplicity of Go’s [net/http](https://pkg.go.dev/net/http) package.

--- 
## Example

```C
#include "http.h"

void simple_handler(response *resp, request *req){
    resp->status_code = HTTP_STATUS_OK;

    if (strcmp(req->uri, "/home") == 0) {
        int fd = open("index.html", O_RDONLY);
        ssize_t n = read_all(fd, resp->body, sizeof(resp->body));
        close(fd);

        add_header(&resp->headers, "Content-Type", "text/html");
        char len_buf[32];
        sprintf(len_buf, "%zd", n);
        add_header(&resp->headers, "Content-Length", len_buf);
        resp->content_length = n;
    }else{
        send_404(resp, req);
    }
}

int main(){
    listen_and_serve("localhost:8080", simple_handler);
}
```

---

## Goals

- [x] Build a basic TCP server from scratch
- [x] Handle multiple clients using `fork()`
- [x] Implement HTTP/1.1 request parsing
- [x] Implement persistent connections (Keep-Alive)
- [ ] Add MIME type detection
- [x] Implement HTTP response helpers
```
void send_404(response *r);
void send_500(response *r);
void send_file(response *r, const char *path);
```
- [x] Improve headers convenience 
- [ ] Add routing system
- [ ] Add logging and configuration
- [ ] Graceful shutdown (SIGINT)
- [x] Explore multithreading with `pthread`
