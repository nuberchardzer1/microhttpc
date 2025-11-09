#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>

#include "rio.h"

#define MAXSIZE 1024

//The rio readn function transfers up to n bytes from the current file 
// position of descriptor fd to memory location usrbuf.
ssize_t rio_read(rio_t *rio, void *usrbuf, size_t n){
    int cnt;
    while (rio->rio_cnt <= 0){
        rio->rio_cnt = read(rio->rio_fd, rio->rio_buf, sizeof(rio->rio_buf));
        if (rio->rio_cnt < 0){
            if (errno != EINTR){
                return -1;
            }
        }else if (rio->rio_cnt == 0)
            return 0;
        else
            rio->rio_bufptr = rio->rio_buf; /* reset buffer ptr */
        
    }

    cnt = (n > rio->rio_cnt) ? rio->rio_cnt : n;
    memcpy(usrbuf, rio->rio_bufptr, cnt);
    rio->rio_bufptr += cnt;
    rio->rio_cnt -= cnt;
    return cnt;
}

/**
 * Waits for a file descriptor to become ready for I/O or timeout.
 *
 * Thin wrapper around poll(2). Ensures at least one byte can be read/written,
 * but subsequent I/O calls may still block if fewer bytes are available.
 *
 * @param fd            File descriptor to wait on.
 * @param events        Events to wait for (POLLIN, POLLOUT, etc).
 * @param timeout_msecs Timeout in milliseconds (-1 = infinite).
 *
 * @return 0 if ready;
 *         -1 on timeout or error (errno = ETIMEDOUT, ECONNRESET, or poll error).
 */
int rio_wait_fd(int fd, short events, int timeout_msecs){
    struct pollfd fds[1];
    int rc;

    fds[0].fd = fd;
    fds[0].events = events;

    rc = poll(fds, 1, timeout_msecs);
    if (rc == 0){
        errno = ETIMEDOUT; 
        return -1; 
    }
    if (rc > 0){
        if (fds[0].revents & events) {
            return 0;
        }
        if (fds[0].revents & (POLLHUP)) {
            errno = ECONNRESET; 
            return -1;
        }
    }
    return -1;
}

ssize_t rio_readline(rio_t *rio, void *usrbuf, int maxlen){
    int n, rc;
    char c, *bufp = usrbuf;
    for (n = 1; n < maxlen; n++){
        if ((rc = rio_read(rio, &c, 1)) == 1){
            *bufp++ = c;
            if (c == '\n'){
                break; 
            }
        } else if (rc == 0){
            if (n == 1){
                return 0; /* EOF, no data read */
            }else{
                break;
            }
        }else{
            return -1;
        }
    }
    *bufp = 0;
    return n;
}

ssize_t rio_readline_with_timeout(rio_t *rio, void *usrbuf, int maxlen, int timeout_msecs){
    int rc = rio_wait_fd(rio->rio_fd, POLLIN, timeout_msecs);
    if (rc < 0){
        return -1;
    }
    return rio_readline(rio, usrbuf, maxlen); //ERROR: BLOCKED IF ARRIVED BYTES < N
}

//The rio writen function transfers n bytes from location usrbuf to descriptor fd.
ssize_t rio_written(rio_t *rio, void *usrbuf, size_t n){
    size_t nleft = n;
    ssize_t nwrite;
    char *bufp = usrbuf;

    while(nleft > 0){
        if((nwrite = write(rio->rio_fd, bufp, nleft) < 0)){
            if (errno == EINTR){
                nwrite = 0;
            }else{
                return -1;
            }
        }else if (nwrite == 0){
            break;
        }
        nleft -= nwrite;
        bufp += nwrite;
    }
    return (n - nleft);
}

ssize_t rio_written_with_timeout(rio_t *rio, void *usrbuf, size_t n, int timeout_msecs){
    int rc = rio_wait_fd(rio->rio_fd, POLLOUT, timeout_msecs);
    if (rc < 0){
        return -1;
    }
    return rio_written(rio, usrbuf, n); //ERROR: CAN BLOCK
}

void rio_readinitb(rio_t *rp, int fd){
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}


ssize_t rio_readn(rio_t *rio, void *usrbuf, size_t n){
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft > 0){
        nread = rio_read(rio, bufp, n);
        if (nread < 0){
            if (errno == EINTR){
                nread = 0;
            }else{
                return -1;
            }
        }else if (nread == 0){
            break;
        }

        bufp += nread;
        nleft -= nread;
    }
    return (n - nleft);
}

ssize_t rio_readn_with_timeout(rio_t *rio, void *usrbuf, size_t n, int timeout_msecs){
    int rc = rio_wait_fd(rio->rio_fd, POLLIN, timeout_msecs);
    if (rc < 0){
        return -1;
    }
    return rio_readn(rio, usrbuf, n); //ERROR: BLOCKED IF ARRIVED BYTES < N
}

ssize_t read_all(int fd, void *usrbuf, ssize_t maxsize){
    size_t cnt = 0;
    ssize_t nread;
    char *bufp = usrbuf;
    
    while(cnt < maxsize){
        nread = read(fd, bufp, MAXSIZE);
        if (nread < 0){
            if (errno == EINTR){
                nread = 0;
            }else{
                return -1;
            }
        }else if (nread == 0){
            break;
        }
        bufp += nread;
        cnt += nread;
    }

    return cnt;
}