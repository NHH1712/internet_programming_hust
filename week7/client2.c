#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void login(int client_socket) {
    char buffer[100], username[50], password[50];
    memset(buffer, '\0', sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);  
    if(strcmp(buffer, "Already logged in") == 0){
        return;
    }
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';  
    send(client_socket, username, strlen(username), 0); 
    do{
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("%s\n", buffer); 
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0'; 
        send(client_socket, password, strlen(password), 0);
        memset(buffer, '\0', sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("%s\n", buffer); 
        if(strcmp(buffer, "Account blocked") == 0){
            return;
        }
    }while(strcmp(buffer, "Login Fail") == 0);
}

void logout(int client_socket) {
    char buffer[100];
    memset(buffer, '\0', sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
    if(strcmp(buffer, "Not logged in") == 0){
        return;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip_address> <port_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port_number = atoi(argv[2]);
    int client_socket;
    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[256];

    // Create socket
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // // Receive data from the server
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        perror("Error receiving data");
    } else {
        buffer[bytes_received] = '\0';
        printf("Server says: %s", buffer);
    }
    while (1) {
        printf("Enter command (login/logout): ");
        char command[50];
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0'; 
        if (strcmp(command, "login") == 0) {
            send(client_socket, "login", strlen("login"), 0);
            login(client_socket);
        } else if (strcmp(command, "logout") == 0) {
            send(client_socket, "logout", strlen("logout"), 0);
            logout(client_socket);
        } else if(strcmp(command, "finish") == 0){
            send(client_socket, "finish", strlen("finish"), 0);
            char buf[100];
            recv(client_socket, buf, sizeof(buf), 0);
            printf("%s\n", buf);
            break;
        } else {
            printf("Invalid command\n");
        }
    }
    // Close the client socket
    close(client_socket);

    return 0;
}
