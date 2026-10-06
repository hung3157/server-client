#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include <unistd.h> 
#include <arpa/inet.h>

#define SERVER_IP "8.8.8.8"
#define PORT 8080

int main() { 

    int sock = socket( AF_INET, SOCK_STREAM, 0);
    if ( sock < 0 ) { 
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(PORT);

    int IP = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    if ( IP <= 0 ) {
        perror("inet_pton() failed");
        exit(EXIT_FAILURE);
    } 
    
    printf("Connecting to %s:%d...\n", SERVER_IP, PORT);

    int connect_status = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if ( connect_status < 0 ) {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to %s:%d\n", SERVER_IP, PORT);

    const char *http_request = "GET / HTTP/1.1\r\nHost: a.com\r\nConnection: close\r\n\r\n";
    ssize_t bytes_sent = send(sock, http_request, strlen(http_request), 0);
    if ( bytes_sent < 0 ) {
        perror("send() failed");
        exit(EXIT_FAILURE);
    }

    char buffer[4096] ={0};
    ssize_t bytes_received;
    while (( bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0) ) > 0) {
        buffer[bytes_received] = '\0'; // Ensure null-termination of the string
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer for the next iteration
    }

    if (bytes_received < 0) {
        perror("Error reading data");
    }

    // 6. Close File Descriptor, release resources
    close(sock);
    printf("Connection closed.\n");

    return 0;
}