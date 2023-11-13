// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <signal.h>
// #include <pthread.h>

// void handle_client(int client_socket, User* userData) {
//     char buffer[100];
//     int loginAttempts = 0;
//     int loggedIn = 0;
//     char username[50];
//     char password[50];

//     while (1) {
//         memset(buffer, '\0', sizeof(buffer));
//         int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
//         if (bytes_received > 0) {
//             buffer[bytes_received] = '\0';
//             printf("Buffer: %s\n", buffer);
//             if (strcmp(buffer, "login") == 0) {
//                 if (loggedIn) {
//                     send(client_socket, "Already logged in", strlen("Already logged in"), 0);
//                 } else {
//                     send(client_socket, "Enter Username", strlen("Enter Username"), 0);
//                     memset(username, '\0', sizeof(username));
//                     recv(client_socket, username, sizeof(username), 0);
//                     printf("User: %s\n", username);
//                     User* currentUser = userData;
//                     while (currentUser != NULL) {
//                         if (strcmp(currentUser->username, username) == 0) {
//                             while (1) {
//                                 send(client_socket, "Enter Password", strlen("Enter Password"), 0);
//                                 char password[50];
//                                 memset(password, '\0', sizeof(password));
//                                 recv(client_socket, password, sizeof(password), 0);
//                                 printf("Pass: %s\n", password);
//                                 if (strcmp(currentUser->password, password) == 0) {
//                                     send(client_socket, "Login Success", strlen("Login Success"), 0);
//                                     loggedIn = 1;
//                                     break;
//                                 } else {
//                                     loginAttempts++;
//                                     if (loginAttempts >= 3) {
//                                         strcpy(currentUser->status, "0");  // Lock the account
//                                         writeData(userData);
//                                         send(client_socket, "Account blocked", strlen("Account blocked"), 0);
//                                         break;
//                                     } else {
//                                         send(client_socket, "Login Fail", strlen("Login Fail"), 0);
//                                         sleep(0.1);
//                                     }
//                                 }
//                             }
//                         }
//                         currentUser = currentUser->next;
//                     }
//                 }
//             } else if (strcmp(buffer, "logout") == 0) {
//                 if (loggedIn) {
//                     loggedIn = 0;
//                     send(client_socket, "Logout Success", strlen("Logout Success"), 0);
//                 } else {
//                     send(client_socket, "Not logged in", strlen("Not logged in"), 0);
//                 }
//             } else {
//                 send(client_socket, "Invalid command", strlen("Invalid command"), 0);
//             }
//         }
//     }
//     close(client_socket);
//     pthread_exit(NULL);
// }

// void sigchld_handler(int signum) {
//     while (waitpid(-1, NULL, WNOHANG) > 0);
// }

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         printf("Usage: %s <PortNumber>\n", argv[0]);
//         return 1;
//     }
//     User* user = NULL;
//     readData(&user);
//     int server_port = atoi(argv[1]);

//     int server_socket, client_socket;
//     struct sockaddr_in server_address, client_address;
//     socklen_t client_address_length = sizeof(client_address);

//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket == -1) {
//         perror("Error opening server socket");
//         exit(1);
//     }

//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(server_port);
//     server_address.sin_addr.s_addr = INADDR_ANY;

//     if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
//         perror("Error binding server socket");
//         exit(1);
//     }

//     if (listen(server_socket, 5) == -1) {
//         perror("Error listening on server socket");
//         exit(1);
//     }

//     printf("Server is listening on port %d...\n", server_port);

//     // Thiết lập xử lý tín hiệu SIGCHLD
//     struct sigaction sa;
//     sa.sa_handler = sigchld_handler;
//     sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
//     sigaction(SIGCHLD, &sa, NULL);

//     while (1) {
//         client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
//         if (client_socket == -1) {
//             perror("Error accepting client connection");
//             continue;
//         }

//         char client_ip[INET_ADDRSTRLEN];
//         inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
//         printf("Client connected from %s:%d\n", client_ip, ntohs(client_address.sin_port));

//         pid_t pid = fork();

//         if (pid == 0) {  // Child process
//             close(server_socket);
//             handle_client(client_socket, user);
//             close(client_socket);
//             exit(0);
//         } else if (pid > 0) {  // Parent process
//             close(client_socket);
//         } else {
//             perror("Error forking");
//         }
//     }

//     close(server_socket);
//     return 0;
// }
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10

typedef struct {
    int socket_fd;
    struct sockaddr_in client_addr;
} client_info;

typedef struct User {
    char username[50];
    char password[50];
    char status[50];
    struct User* next;
} User;

User* userData = NULL;

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

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    char buffer[256];
    int loginAttempts = 0;
    int loggedIn = 0;
    char username[50];
    char password[50];

    char client_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client->client_addr.sin_addr), client_address, INET_ADDRSTRLEN);
    printf("Client connected from %s:%d\n", client_address, ntohs(client->client_addr.sin_port));
    const char *success_message = "Connection successful. Welcome to the server!\n";
    if (send(client->socket_fd, success_message, strlen(success_message), 0) == -1) {
        perror("Error sending success message");
    }
    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        int bytes_received = recv(client->socket_fd, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Buffer: %s\n", buffer);
            if (strcmp(buffer, "login") == 0) {
                if (loggedIn) {
                    send(client->socket_fd, "Already logged in", strlen("Already logged in"), 0);
                } else {
                    send(client->socket_fd, "Enter Username", strlen("Enter Username"), 0);
                    memset(username, '\0', sizeof(username));
                    recv(client->socket_fd, username, sizeof(username), 0);
                    printf("User: %s\n", username);
                    User* currentUser = userData;
                    while (currentUser != NULL) {
                        if (strcmp(currentUser->username, username) == 0) {
                            while(1){
                                send(client->socket_fd, "Enter Password", strlen("Enter Password"), 0);
                                char password[50];
                                memset(password, '\0', sizeof(password));
                                recv(client->socket_fd, password, sizeof(password), 0);
                                printf("Pass: %s\n", password);
                                if (strcmp(currentUser->password, password) == 0) {
                                    send(client->socket_fd, "Login Success", strlen("Login Success"), 0);
                                    loggedIn = 1;
                                    break;
                                } else {
                                    loginAttempts++;
                                    if (loginAttempts >= 3) {
                                        strcpy(currentUser->status, "0");  // Lock the account
                                        writeData(userData);
                                        send(client->socket_fd, "Account blocked", strlen("Account blocked"), 0);
                                        break;
                                    }else{
                                        send(client->socket_fd, "Login Fail", strlen("Login Fail"), 0);
                                        sleep(0.1);
                                    }
                                }
                            }
                        }
                        currentUser = currentUser->next;
                    }
                }
            } else if (strcmp(buffer, "logout") == 0) {
                if (loggedIn) {
                    loggedIn = 0;
                    send(client->socket_fd, "Logout Success", strlen("Logout Success"), 0);
                } else {
                    send(client->socket_fd, "Not logged in", strlen("Not logged in"), 0);
                }
            } else if(strcmp(buffer, "finish") == 0){
                send(client->socket_fd, "Finish Program", strlen("Finish Program"), 0);
                close(client->socket_fd);
            } else {
                send(client->socket_fd, "Invalid command", strlen("Invalid command"), 0);
            }
        }
    }
    close(client->socket_fd);
    free(client);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    readData(&userData);
    int port_number = atoi(argv[1]);
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id[MAX_CLIENTS];
    client_info *clients[MAX_CLIENTS];

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    // Bind to port_number
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port_number);

    while (1) {
        // Accept a connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Create a client_info structure to pass to the thread
        client_info *client = (client_info *)malloc(sizeof(client_info));
        client->socket_fd = client_socket;
        client->client_addr = client_addr;

        // Create a new thread to handle the client
        if (pthread_create(&thread_id[MAX_CLIENTS], NULL, handle_client, (void *)client) != 0) {
            perror("Error creating thread");
            close(client_socket);
            free(client);
            continue;
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}

