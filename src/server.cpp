#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080

#define BUFFER_SIZE 1024

int main() {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    // AF_INET la de tao ket noi IPv4, AF_INET6 la de tao ket noi IPv6
    // SOCK_STREAM la de tao ket noi TCP, SOCK_DGRAM la de tao ket noi UDP
    // 0 la de chon giao thuc mac dinh (TCP cho SOCK_STREAM, UDP cho SOCK_DGRAM)


    if (server_fd < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    int reuse_address = 1; // 1 la de bat option_name, 0 la de tat

    //int setsockopt(int socket, int level, int option_name,  --- level (SOL_SOCKET, IPPROTO_TCP, IPPROTO_IP) >> option_name
    //    const void *option_value, socklen_t option_len);
    
    // cai nay la de thiet lap lai cau hinh cua socket 

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address)) < 0) {
        perror("setsockopt() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    } // tranh loi "Address already in use" khi restart server

    if (setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &reuse_address, sizeof(reuse_address)) < 0) {
        perror("setsockopt() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    } // de giu ket noi khi client bi mat ket noi, de tranh loi "Broken pipe" khi gui du lieu

    struct timeval timeout;
    timeout.tv_sec = 60;  // 60 giây
    timeout.tv_usec = 0; // micro giây

    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    } // cai dat thoi gian timeout cho viec nhan du lieu tu client, tranh tinh trang treo server khi client khong gui du lieu


    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
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



    signal(SIGCHLD, SIG_IGN); // tu dong xoa process con, SIG_IGN la de bo qua tin hieu SIGCHLD

    while (1) {
        struct sockaddr_in client_addr = {};
        socklen_t client_addr_len = sizeof(client_addr);


        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            if (errno == EINTR) { // errno  la bien toan cuc, eintr la system interrupted => cho n tiep tuc vong lap
                continue;
            }
            perror("accept() failed");
            break;
        }

        pid_t child_pid = fork();
        // fork() de tao process con, process con se xu ly ket noi client, process cha se tiep tuc cho ket noi moi
        // child_pid < 0: fork() failed, child_pid == 0: process con, child_pid > 0: process cha

        if (child_pid < 0) {
            perror("fork() failed");
            close(client_fd);
            continue;
        }

        if (child_pid == 0) {

            close(server_fd);
            printf("Client connected.\n");

            // echo 
            
            char buffer[BUFFER_SIZE];

            ssize_t bytes_received;

            while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {

                ssize_t total_sent = 0;
                while (total_sent < bytes_received) {
                    ssize_t bytes_sent = send(client_fd, buffer + total_sent, bytes_received - total_sent, 0);
                    
                    if (bytes_sent < 0) {
                        if (errno == EINTR) {
                            continue;
                        }
                        perror("send() failed");
                        close(client_fd);
                        _exit(EXIT_FAILURE);
                    }

                    total_sent += bytes_sent;
                }
            }

            if (bytes_received < 0) {
                perror("recv() failed");
            }

            close(client_fd);
            _exit(EXIT_SUCCESS);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}