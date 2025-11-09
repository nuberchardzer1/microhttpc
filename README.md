# microhttpc

A **minimal and educational HTTP web framework** written in pure C using POSIX sockets.

It provides a simple foundation for building web servers and handling routing, from managing raw TCP connections to parsing HTTP requests and generating responses.

Inspired by the simplicity of Go’s [net/http](https://pkg.go.dev/net/http) package.

I`ll be glad to any ideas or contributions — feel free to open an issue or PR!

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
    }else{
        send_404(resp, req);
    }
}

int main(){
    listen_and_serve("localhost:8080", simple_handler);
}
```
---

## How It Works

`microhttpc` provides:

- A **blocking TCP server** using `accept()` and `pthread` for concurrency.  
- A simple **HTTP/1.1 parser** that fills a `request` struct with the method, URI, headers, and body.  
- A lightweight **header map** for O(1) lookups.  
- A `handle_func` callback that receives both `request *req` and `response *resp`.  
  Inside the handler, you fill the `response` — set `status_code`, add headers, and write the body.  
  The framework automatically serializes and sends it back to the client.  
- Built-in helpers like `send_404()`, `send_file()`, and `send_500()`.  
- Optional **Keep-Alive** and per-connection **read timeouts**.  

No external dependencies — just pure C and POSIX.

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
