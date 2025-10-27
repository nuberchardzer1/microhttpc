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

ssize_t rio_read(rio_t *rio, void *usrbuf, size_t n);
ssize_t rio_written(rio_t *rio, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rio, int fd);
ssize_t rio_readn(rio_t *rio, void *usrbuf, size_t n);
ssize_t rio_readline(rio_t *rio, void *usrbuf, int maxlen);

#endif