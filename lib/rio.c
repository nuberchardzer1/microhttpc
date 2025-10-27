#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "rio.h"
#include <string.h>

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

//The rio writen function transfers n bytes from location usrbuf to descriptor fd.
ssize_t rio_written(rio_t *rio, void *usrbuf, size_t n){
    size_t nleft = n;
    ssize_t nwrite;
    char *bufp = usrbuf;

    while(nwrite > 0){
        if((nwrite = write(rio->rio_fd, bufp, nleft))){
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