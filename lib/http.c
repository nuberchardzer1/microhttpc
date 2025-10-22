#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct server {
    char *addr;
    struct sockaddr_in server_addr;
} server;


int serve(server srv);
int handle_connection(server srv, int conn_fd); 
int parse_sockaddr(char *addr_str, struct sockaddr_in *server_addr);

/// @brief Starts a TCP server that listens on the specified address and begins serving connections.
/// 
/// This function initializes a TCP socket, binds it to the given address, and 
/// launches the main serving loop through the `serve()` function.  
/// The address should be provided in the format `"host:port"`, for example `"127.0.0.1:8080"`.
///
/// @param addr A string specifying the address and port for the server to listen on (e.g. "0.0.0.0:8080").
/// @return Returns `0` on success, or `-1` if the server setup (socket creation, binding, or parsing) fails.
int listen_and_serve(char *addr){
    struct sockaddr_in server_addr;

    if (parse_sockaddr(addr, &server_addr) < 0){
        return -1;
    }

    server srv = { addr, server_addr };
    serve(srv);
    return 0;
}

int serve(server srv){
    int sockfd, conn; 

    printf("Server is running...\n");

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    printf("socket: success %d\n", sockfd);

     /**
     * This binds the socket descriptor to the server thus enabling the server
     * to listen for connections and communicate with other clients
     */
    if (bind(sockfd, (struct sockaddr *)&srv.server_addr, sizeof(srv.server_addr)) < 0){
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) < 0) {
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening...\n");

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
            handle_connection(srv, conn);
            close(conn);
            exit(0);
        }
        sleep(5);
    }
    return 0;
}


int handle_connection(server srv, int conn_fd) {
    char buf[1024];
    while (1) {
        int n = recv(conn_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buf[n] = '\0';
        printf("Client: %s\n", buf);

        char msg[1024];
        snprintf(msg, sizeof(msg), "FROM SERVER: %s", buf);
        send(conn_fd, msg, strlen(msg), 0);
    }

    close(conn_fd);
    return 0;
}

int parse_sockaddr(char *addr_str, struct sockaddr_in *server_addr){
    while (*addr_str++ != ':') 
        ;

    int port = atoi(addr_str);
    (*server_addr).sin_family = AF_INET; 
    (*server_addr).sin_port = htons(port);
    (*server_addr).sin_addr.s_addr = htonl(INADDR_ANY); // TODO: PARSE HOST ADDR
    return 0;
}