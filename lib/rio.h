#ifndef RIO_H
#define RIO_H

#include <stdio.h>

#define RIO_BUFSIZE 8192

typedef struct rio_t{
    int rio_fd; /* descriptor for this internal buf */
    int rio_cnt; /* unread bytes in internal buf */
    char *rio_bufptr; /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

void rio_readinitb(rio_t *rio, int fd);
ssize_t rio_written(rio_t *rio, void *usrbuf, size_t n);
ssize_t rio_written_with_timeout(rio_t *rio, void *usrbuf, size_t n, int timeout_msecs);

ssize_t rio_readn(rio_t *rio, void *usrbuf, size_t n);
ssize_t rio_readn_with_timeout(rio_t *rio, void *usrbuf, size_t n, int timeout_msecs);

ssize_t rio_readline(rio_t *rio, void *usrbuf, int maxlen);
ssize_t rio_readline_with_timeout(rio_t *rio, void *usrbuf, int maxlen, int timeout_msecs);

ssize_t read_all(int fd, void *usrbuf, ssize_t maxsize);

#endif