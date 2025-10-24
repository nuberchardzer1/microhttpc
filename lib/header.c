#include <string.h>
#include <ctype.h>
#include "http.h"

int read_header(char *s, char *key, char *val);

int read_headers(rio_t *rio, header_t *headers, int maxcnt){
    int i, n;
    int cnt = 0;
    char line[HTTP_MAX_LINE_LEN]; // redundant alloc? 
    header_t hdr; 

    while(line[0] != '\n' && line[1] != '\n'){
        if (cnt >= maxcnt) // too many headers
            return -1;

        n = rio_readline(rio, line, HTTP_MAX_LINE_LEN);
        
        if (read_header(line, hdr.key, hdr.val) == -1){
            continue;
        }

        headers[cnt++] = hdr;
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

int scan(char *key, header_t *headers, int n){
    int i;

    for (i = 0; i < n; i++){
        if (strcmp(headers[i].key, key) == 0){
            return i;
        }
    }
    return -1;
}

int get_header_pos(char *key, header_t *headers, int n){
    return scan(key, headers, n);
}