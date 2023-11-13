#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

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
    // fgets(password, sizeof(password), stdin);
    // password[strcspn(password, "\n")] = '\0';  
    // send(client_socket, password, strlen(password), 0);
    // recv(client_socket, buffer, sizeof(buffer), 0);
    // printf("%s\n", buffer); 
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
        printf("Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        return 1;
    }
    int client_socket;
    struct sockaddr_in server_address;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error opening client socket");
        exit(1);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_address.sin_addr);
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error connecting to server");
        exit(1);
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
        } else {
            printf("Invalid command\n");
        }
    }
    close(client_socket);
    return 0;
}

