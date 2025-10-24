#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "http.h"
#include "rio.h"

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr_in sa_in;

int http_listen(server srv, int *sockfd);
int serve(server srv, int sockfd);
int handle_connection(server srv, int conn_fd); 
int parse_sockaddr(char *addr_str, sa_in *server_addr);
int serve_http(int fd, server srv, request req);

/// @brief Starts a TCP server that listens on the specified address and begins serving connections.
/// 
/// This function initializes a TCP socket, binds it to the given address, and 
/// launches the main serving loop through the `serve()` function.  
/// The address should be provided in the format `"host:port"`, for example `"127.0.0.1:8080"`.
///
/// @param addr A string specifying the address and port for the server to listen on (e.g. "0.0.0.0:8080").
/// @return Returns `0` on success, or `-1` if the server setup (socket creation, binding, or parsing) fails.
int listen_and_serve(char *addr){
    int sockfd;
    sa_in server_addr;
    if (parse_sockaddr(addr, &server_addr) < 0){
        return -1;
    }

    server srv = { addr, server_addr };

    http_listen(srv, &sockfd);
    serve(srv, sockfd);
    return 0;
}

int http_listen(server srv, int *sockfd){
    printf("Server is running...\n");

    if (((*sockfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    printf("socket: success %d\n", (*sockfd));

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
    printf("Server is listening...\n");
}

int serve(server srv, int sockfd){
    int conn, rc; 

    while(1){
        conn = accept(sockfd, (struct sockaddr *)NULL, NULL);
        if (conn < 0){
            printf("conn < 0");
            perror("Conn Accept Failed");
            exit(EXIT_FAILURE);
        }

        printf("New conn!\n");

        pid_t pid = fork();
        if (pid == 0){
            rc = handle_connection(srv, conn);
            close(conn);
            exit(0);
        }else if (pid > 0) 
            close(conn); 
         }
    return 0;
}


int handle_connection(server srv, int conn_fd) {
    request req;
    int rc = 0;
    rio_t rp;
    rio_readinitb(&rp, conn_fd);
    while (1) {
        rc = read_request(&rp, &req);
        if (rc <= 0){
            break;
        }
    }
    return rc;
}

int parse_sockaddr(char *addr_str, struct sockaddr_in *server_addr){
    while (*addr_str++ != ':') 
        ;

    int port = atoi(addr_str);
    (*server_addr).sin_family = AF_INET; 
    (*server_addr).sin_port = htons(port);
    (*server_addr).sin_addr.s_addr = htonl(INADDR_ANY); 
    return 0;
}