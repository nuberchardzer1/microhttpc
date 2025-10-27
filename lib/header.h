#ifndef HEADER_H
#define HEADER_H

#include "rio.h"

typedef struct header_t{
    char key[64];
    char val[256];
} header_t;

typedef header_t headers_t[128];

int read_headers(rio_t *rio, header_t *headers, int maxcnt);
int get_header_pos(char *key, header_t *headers, int n);

#endif