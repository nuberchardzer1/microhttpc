#include <netinet/in.h>
#include "rio.h"
#include "header.h"

#define HTTP_MAX_LINE_LEN 1024
#define HTTP_MAX_HEADERS 128

#define MAXPAYLOAD 8192

#define NOT_FOUND "Not Found"
#define INTERNAL_ERROR "Internal Server Error"

enum {
    HTTP_STATUS_OK = 200, // RFC 9110, 15.3.1
    HTTP_STATUS_MOVED_PERMANENTLY = 301, // RFC 9110, 15.4.2
    HTTP_STATUS_FOUND = 302, // RFC 9110, 15.4.3
    HTTP_STATUS_BAD_REQUEST = 400, // RFC 9110, 15.5.1
    HTTP_STATUS_UNAUTHORIZED = 401, // RFC 9110, 15.5.2
    HTTP_STATUS_FORBIDDEN = 403, // RFC 9110, 15.5.4
    HTTP_STATUS_NOT_FOUND = 404, // RFC 9110, 15.5.5
    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500, // RFC 9110, 15.6.1
    HTTP_STATUS_BAD_GATEWAY = 502, // RFC 9110, 15.6.3
    HTTP_STATUS_SERVICE_UNAVAILABLE = 503, // RFC 9110, 15.6.4
    HTTP_STATUS_GATEWAY_TIMEOUT = 504  // RFC 9110, 15.6.5
};

// A request represents the server side of an HTTP request.
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

// A response represents the server side of an HTTP response.
typedef struct response {

    request *req; // Request for this response

    int status_code; //Status code 
    char status_text[32];

    char content_type[32];

    headers_t headers;
    int headers_cnt;
    
    // Body stores the response payload
    char body[MAXPAYLOAD];

    size_t content_length;

} response;

typedef void (*handle_func)(response *resp, request *req);

typedef struct server{
    char *addr;
    struct sockaddr_in server_addr;
    handle_func handle;
} server;

int read_request(rio_t *rio, request *req); 

// listen_and_serve: listen socket and handles http requests
int listen_and_serve(char *addr, handle_func);
int marshall_response(response *resp, char *usrbuf, size_t size);

void send_404(response *resp, request *req);
void send_500(response *resp);
void redirect(response *resp, int code, char *location);