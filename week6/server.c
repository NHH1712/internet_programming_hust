#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>

typedef struct User {
    char username[50];
    char password[50];
    char status[50];
    struct User* next;
} User;

void readData(User** userData){
    FILE* file = fopen("account.txt", "r");
    if (file == NULL){
        return;
    }
    char line[200];
    User* tail = NULL;
    while (fgets(line, sizeof(line), file)){
        User* user = (User*)malloc(sizeof(User));
        sscanf(line, "%s %s %s", user->username, user->password, user->status);
        user->next = NULL;

        if (*userData == NULL){
            *userData = user;
            tail = user;
        }else{
            tail->next = user;
            tail = user;
        }
    }
    fclose(file);
}
void writeData(User* userData){
    FILE* file = fopen("account.txt", "w");
    if (file == NULL){
        return;
    }
    User* currentData = userData;
    while (currentData != NULL){
        fprintf(file, "%s %s %s\n", currentData->username, currentData->password, currentData->status);
        currentData = currentData->next;
    }
    fclose(file);
}
int findAccount(User* userData, char* username){
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            return 1;
        }
        currentData = currentData->next;
    }
    return 0;
}
int login(User* userData, char* username, char* password){
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            if(strcmp(currentData->password, password) == 0){
                if(strcmp(currentData->status, "1") == 0){
                    return 3;
                }else{
                    return 2;
                }
            }else{
                return 1;
            }
        }
        currentData = currentData->next;
    }
    return 0;
}
void blockAccount(User* userData, char* username){
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            strcpy(currentData->status, "0");
            writeData(userData);
        }
        currentData = currentData->next;
    }
    return;
}

void handle_client(int client_socket, User* userData) {
    char buffer[100];
    int loginAttempts = 0;
    int loggedIn = 0;
    char username[50];
    char password[50];
    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Buffer: %s\n", buffer);
            if (strcmp(buffer, "login") == 0) {
                if (loggedIn) {
                    send(client_socket, "Already logged in", strlen("Already logged in"), 0);
                } else {
                    send(client_socket, "Enter Username", strlen("Enter Username"), 0);
                    memset(username, '\0', sizeof(username));
                    recv(client_socket, username, sizeof(username), 0);
                    printf("User: %s\n", username);
                    User* currentUser = userData;
                    while (currentUser != NULL) {
                        if (strcmp(currentUser->username, username) == 0) {
                            while(1){
                                send(client_socket, "Enter Password", strlen("Enter Password"), 0);
                                char password[50];
                                memset(password, '\0', sizeof(password));
                                recv(client_socket, password, sizeof(password), 0);
                                printf("Pass: %s\n", password);
                                if (strcmp(currentUser->password, password) == 0) {
                                    send(client_socket, "Login Success", strlen("Login Success"), 0);
                                    loggedIn = 1;
                                    break;
                                } else {
                                    // send(client_socket, "Login Fail", strlen("Login Fail"), 0);
                                    // sleep(1);
                                    loginAttempts++;
                                    if (loginAttempts >= 3) {
                                        strcpy(currentUser->status, "0");  // Lock the account
                                        writeData(userData);
                                        send(client_socket, "Account blocked", strlen("Account blocked"), 0);
                                        // close(client_socket);
                                        break;
                                    }else{
                                        send(client_socket, "Login Fail", strlen("Login Fail"), 0);
                                        sleep(0.1);
                                    }
                                }
                            }
                        }
                        currentUser = currentUser->next;
                    }
                    // if (!loggedIn) {
                    //     send(client_socket, "User not found", strlen("User not found"), 0);
                    // }
                }
            } else if (strcmp(buffer, "logout") == 0) {
                if (loggedIn) {
                    loggedIn = 0;
                    send(client_socket, "Logout Success", strlen("Logout Success"), 0);
                } else {
                    send(client_socket, "Not logged in", strlen("Not logged in"), 0);
                }
            } else {
                send(client_socket, "Invalid command", strlen("Invalid command"), 0);
            }
        }
    }
}

void sigchld_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }
    User* user = NULL;
    readData(&user);
    int server_port = atoi(argv[1]);

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error opening server socket");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding server socket");
        exit(1);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Error listening on server socket");
        exit(1);
    }

    printf("Server is listening on port %d...\n", server_port);

    // Thiết lập xử lý tín hiệu SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket == -1) {
            perror("Error accepting client connection");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client connected from %s:%d\n", client_ip, ntohs(client_address.sin_port));

        pid_t pid = fork();

        if (pid == 0) {  // Child process
            close(server_socket);
            handle_client(client_socket, user);
            close(client_socket);
            exit(0);
        } else if (pid > 0) {  // Parent process
            close(client_socket);
        } else {
            perror("Error forking");
        }
    }

    close(server_socket);
    return 0;
}
