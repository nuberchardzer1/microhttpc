#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "http.h"

int read_header(char *s, char *key, char *val);

int read_headers(rio_t *rio, header_map_t *map, int maxcnt){
    int i, n;
    int cnt = 0;
    char line[HTTP_MAX_LINE_LEN]; // redundant alloc? 
    header_t header; 

    while(line[0] != '\n' && line[1] != '\n'){
        if (cnt >= maxcnt) // too many headers
            return -1;

        n = rio_readline(rio, line, HTTP_MAX_LINE_LEN);
        
        if (read_header(line, header.key, header.val) == -1){
            continue;
        }

        add_header(map, header.key, header.val);
    }
    return 0;
}

int read_header(char *s, char *key, char *val){
    int i;
    int del_pos = -1;

    while (isspace((unsigned char)*s))
        s++;

    for (i = 0; s[i] != '\0'; i++) { // search delimiter index // enificcient
        if (s[i] == ':') {
            del_pos = i;
            break;
        }
    }

    if (del_pos == -1){
        return -1;
    }

    memcpy(key, s, del_pos);
    key[del_pos] = '\0';

    s += del_pos + 1; //skip delimiter
    while (isspace((unsigned char)*s))
        s++;

        
    const char *end = s + strlen(s);
    while (end > s && (end[-1] == '\r' || end[-1] == '\n' || isspace((unsigned char)end[-1])))
        end--;

    size_t val_len = end - s;
    memcpy(val, s, val_len);
    val[val_len] = '\0';

    return 0;
}

// DJB2 Hash
unsigned long hash(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = hash * 33 + c;

    return hash;
}

void add_header(header_map_t *map, char *key, char *val){
    unsigned long idx = hash(key) % HEADER_BUCKETS;

    header_t *new_node = (header_t *)malloc(sizeof(header_t));
    strncpy(new_node->key, key, sizeof(new_node->key) - 1);
    strncpy(new_node->val, val, sizeof(new_node->val) - 1);
    new_node->next = NULL;

    header_t *node = map->buckets[idx];

    if (node == 0){
        map->buckets[idx] = new_node;
        return;
    }

    if (strcmp(node->key, key) == 0) {
        strncpy(node->val, val, sizeof(node->val) - 1);
        free(new_node);
        return;
    }

    header_t *cur = node;
    while (cur->next && strcmp(cur->next->key, key) != 0) {
        cur = cur->next;
    }
    
    if (cur->next) {
        strncpy(cur->next->val, val, sizeof(cur->next->val) - 1);
        free(new_node); // key already exists
    } else {
        cur->next = new_node;
    }
}

char *get_header(header_map_t *map, char *key){
    unsigned long idx = hash(key) % HEADER_BUCKETS;
    header_t *node = map->buckets[idx];
    while (node){
        if (strcmp(node->key, key) == 0){
            return node->val;
        }
        node = node->next;
    }
    return NULL;
}