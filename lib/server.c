#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <linux/time.h>

#include "http.h"
#include "rio.h"

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr_in sa_in;

int http_listen(server srv, int *sockfd);
int serve(server *srv, int sockfd);
void *handle_connection(void *conn_p); 
int parse_port(char *addr_str);
int serve_http(int fd, server srv, request req);

/// @brief Starts a TCP server that listens on the specified address and begins serving connections.
/// 
/// This function initializes a TCP socket, binds it to the given address, and 
/// launches the main serving loop through the `serve()` function.  
/// The address should be provided in the format `"host:port"`, for example `"127.0.0.1:8080"`.
///
/// @param addr A string specifying the address and port for the server to listen on (e.g. "0.0.0.0:8080").
/// @return Returns `0` on success, or `-1` if the server setup (socket creation, binding, or parsing) fails.
int listen_and_serve(char *addr, handle_func f){
    int sockfd, port;
    sa_in server_addr;

    port = parse_port(addr);
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

    server srv = { 
        .addr = addr, 
        .server_addr = server_addr, 
        .handle = f, 
        .read_timeout = 60,
    };

    printf("Server is listening on port: %d\n", port);
    http_listen(srv, &sockfd);
    serve(&srv, sockfd);
    return 0;
}

int http_listen(server srv, int *sockfd){
    if (((*sockfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

     /**
     * This binds the socket descriptor to the server thus enabling the server
     * to listen for connections and communicate with other clients
     */
    if (bind((*sockfd), (const struct sockaddr *)&srv.server_addr, sizeof(srv.server_addr)) < 0){
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    if (listen((*sockfd), 5) < 0) {
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int serve(server *srv, int sockfd){
    int conn_fd; 
    pthread_t thread;
    while(1){
        conn_fd = accept(sockfd, (struct sockaddr *)NULL, NULL);
        if (conn_fd < 0){
            printf("conn < 0");
            perror("Conn Accept Failed");
            exit(EXIT_FAILURE);
        }
        conn *cc = malloc(sizeof(conn));
        if (!cc) {
            perror("malloc failed");
            close(conn_fd);
            continue;
        }
        cc->server = srv;
        cc->conn_fd = conn_fd; 
        printf("new conn\n");
        pthread_create(&thread, NULL, handle_connection, cc);
    }
}

/**
 * Thread entry point for handling a single client connection.
 * Expected to be started via pthread_create().
 *
 * @param conn_p (void *) pointer to a `conn` struct
 * @return NULL always
 */
void *handle_connection(void *conn_p) {
    conn *cc = (conn *)conn_p;
    char resp_buf[MAXPAYLOAD];
    
    int n, rc;
        
    request req = {0};
    response resp = {0};
    resp.req = &req;
    
    rio_t rp;
    rio_readinitb(&rp, cc->conn_fd);

    pthread_t tid;
    pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    read_req_thread_ctx req_ctx;
    req_ctx.rio = &rp;
    req_ctx.req = &req;
    req_ctx.done = &cond;

    struct timespec ts;

    while (1){
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += cc->server->read_timeout;

        pthread_create(&tid, NULL, read_request_thread, &req_ctx);

        rc = pthread_cond_timedwait(&cond, &mu, &ts);
        if (rc == ETIMEDOUT){
            printf("Timeout expired\n");
            pthread_cancel(tid);
            break;
        }

        if (req_ctx.rc < 0) {
            char reason[] = "Bad Request";
            send_http_error(resp_buf, HTTP_STATUS_BAD_REQUEST, reason, reason);
            printf("read request error: %d\n", rc);
            break;
        }

        cc->server->handle(&resp, &req);

        n = marshall_response(&resp, resp_buf, sizeof(resp_buf));
        if (rio_written(&rp, resp_buf, n) < 0) {
            printf("write error\n");
            break;
        }

        char *conn_hdr = get_header(&req.headers, "Connection");
        if (conn_hdr == NULL || strcmp(conn_hdr, "keep-alive") != 0){
            if (conn_hdr) {
                printf("Connection: %s\n", conn_hdr);
                break;
            } 
        }
        conn_hdr = get_header(&resp.headers, "Connection");
        if (conn_hdr != NULL){
            if (strcmp(conn_hdr, "close") == 0){
                break;
            }
        }

        free_header_map(&req.headers);
        free_header_map(&resp.headers);
    }

    printf("Close conn\n");
    close(cc->conn_fd);
    free(cc);
    free_header_map(&req.headers);
    free_header_map(&resp.headers);
    return NULL;
}

int parse_port(char *addr_str){
    while (*addr_str++ != ':') 
        ;
    return atoi(addr_str);
}