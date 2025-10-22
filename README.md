# simple-web-framework-in-c

A **minimal and educational HTTP web framework** written in pure C using POSIX sockets.

This project is designed to **teach and demonstrate** how HTTP servers and routing work at a low level — from handling raw TCP connections to parsing HTTP requests and building responses.  
Inspired by the simplicity of Go’s [net/http](https://pkg.go.dev/net/http) package.


---

## 🧠 Learning Goals

- [x] Build a basic TCP server from scratch
- [x] Handle multiple clients using `fork()`
- [ ] Implement HTTP/1.1 request parsing
- [ ] Add routing system
- [ ] Serve static files
- [ ] Add logging and configuration
- [ ] Graceful shutdown (SIGINT)
- [ ] Explore multithreading with `pthread`
