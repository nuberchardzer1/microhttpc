#ifndef REQUEST_H
#define REQUEST_H

#include "header.h"

#define MAXPAYLOAD 8192

typedef struct request{
    // Method specifies the HTTP method (GET, POST, PUT, etc.).
	// For client requests, an empty string means GET.
    char method[16];

    // Header contains the request header fields either received
	// by the server or to be sent by the client.
    headers_t headers;

    // RequestURI is the unmodified request-target of the
	// Request-Line as sent by the client to a server.
    char uri[256];

    // The protocol version for incoming server requests.
    char proto[32];

    // ContentLength records the length of the associated content.
	// The value -1 indicates that the length is unknown.
	// Values >= 0 indicate that the given number of bytes may
	// be read from Body.
    int content_length;

    // Body stores the request payload
    char body[MAXPAYLOAD];

} request;

int read_request(rio_t *rio, request *req); 

#endif