#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr; 
    struct sockaddr_in client_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_fd < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family  = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if ( bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ( listen(server_fd, 5) < 0) {
        perror("listen() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);


    socklen_t client_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("accept() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    char buffer[BUFFER_SIZE] = {0};
    /*
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received < 0) {
        perror("recv() failed");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_received] = '\0'; // Ensure null-termination of the string
    printf("Received data:\n%s\n", buffer);
    */ 

    ssize_t bytes_received;
    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Ensure null-termination of the string
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer for the next iteration
    }

    close(client_fd);
    close(server_fd);
    printf("Connection closed.\n");

    return 0;
}