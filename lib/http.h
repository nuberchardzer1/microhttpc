#include <netinet/in.h>
#include "rio.h"
#include "request.h"

#define HTTP_MAX_LINE_LEN 1024
#define HTTP_MAX_HEADERS 128

typedef struct {
    char *addr;
    struct sockaddr_in server_addr;
} server;

int listen_and_serve(char *addr);
