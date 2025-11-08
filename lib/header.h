#ifndef HEADER_H
#define HEADER_H

#include "rio.h"

#define HEADER_BUCKETS 1024

typedef struct header_t{
    char key[64];
    char val[256];
    struct header_t *next;
} header_t;

typedef struct header_map_t{
    header_t *buckets[HEADER_BUCKETS];
} header_map_t;

int read_headers(rio_t *rio, header_map_t *headers, int maxcnt);
void add_header(header_map_t *map, char *key, char *val);
char *get_header(header_map_t *map, char *key); 

#endif