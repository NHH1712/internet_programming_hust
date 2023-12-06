#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <poll.h>
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

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
    FILE* file = fopen("nguoidung.txt", "r");
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
    FILE* file = fopen("nguoidung.txt", "w");
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
void changePass(User* userData, char* username, char* newPassword){
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            strcpy(currentData->password, newPassword);
            writeData(userData);
        }
        currentData = currentData->next;
    }
    return;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    User* user = NULL;
    readData(&user);
    int port_number = atoi(argv[1]);
    int serverSocket, clientSockets[MAX_CLIENTS], maxClients = MAX_CLIENTS;
    struct pollfd fds[MAX_CLIENTS + 1]; 
    fd_set readfds;
    int activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t addrlen = sizeof(clientAddress);

    char server_message[200];
    char client_account[200], client_password[200], client_newpassword[200];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_account, '\0', sizeof(client_account));
    memset(client_password, '\0', sizeof(client_password));
    memset(client_newpassword, '\0', sizeof(client_newpassword));

    for (i = 0; i < maxClients; i++) {
        clientSockets[i] = 0;
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port_number);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error binding socke");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 3) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d ... \n", port_number);

    while (1) {
        fds[0].fd = serverSocket;
        fds[0].events = POLLIN;

        for (i = 0; i < maxClients; i++) {
        sd = clientSockets[i];
        fds[i + 1].fd = sd;
        fds[i + 1].events = POLLIN;
        }

        activity = poll(fds, max_sd + 1, -1); 
        if ((activity < 0) && (errno != EINTR)) {
            printf("Lỗi poll");
        }
        if (fds[0].revents & POLLIN) {
           int newSocket;
            if ((newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addrlen)) < 0) {
                perror("Lỗi chấp nhận kết nối");
                exit(EXIT_FAILURE);
            }
            printf("Kết nối mới, socket fd: %d, IP: %s, Port: %d\n", newSocket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
            for (i = 0; i < maxClients; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    break;
                }
            }
        }

        for (i = 0; i < maxClients; i++) {
            sd = clientSockets[i];
            if (fds[i].revents & POLLIN) {
                while(1){
                    if (recv(sd, client_account, sizeof(client_account), 0) < 0){
                        printf("Couldn't receive\n");
                        return -1;
                    }
                    printf("Account: %s\n", client_account);
                    if(strcmp(client_account, "NULL") == 0){
                        break;
                    }
                    if(findAccount(user, client_account) == 1){
                        strcpy(server_message, "Insert password");
                        if (send(sd, server_message, strlen(server_message), 0) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                    }else{
                        strcpy(server_message, "Not found account");
                        if (send(sd, server_message, strlen(server_message), 0) < 0){
                            printf("Can't send\n");
                            return -1;
                        }
                        // close(sd);
                        // close(socket_desc);
                    }
                    int count = 0;
                    int login_successful = 0;
                    while(!login_successful && count < 3){
                        memset(client_password, '\0', sizeof(client_password));
                        if(recv(sd, client_password, sizeof(client_password), 0) < 0){
                            printf("Couldn't receive\n");
                            return -1;
                        }
                        printf("Password: %s\n", client_password);
                        if(strcmp(client_password, "NULL") == 0){
                            return 0;
                        }
                        if(login(user, client_account, client_password) == 3){
                            strcpy(server_message, "OK");
                            login_successful = 1;
                        }else if(login(user, client_account, client_password) == 2){
                            strcpy(server_message, "Account not ready");
                        }else if(login(user, client_account, client_password) == 1){
                            strcpy(server_message, "Not OK");
                        }else{
                            strcpy(server_message, "Account is blocked");
                        }
                        printf("Server message: %s\n", server_message);
                        count++;
                        if(count >= 3){
                            blockAccount(user, client_account);
                            strcpy(server_message, "Account is blocked");
                        }
                        if (send(sd, server_message, strlen(server_message), 0)< 0){
                            printf("Can't send\n");
                            return -1;
                        }
                    }
                    if (count >= 3) {
                        break;
                    }
                    if (login_successful) {
                        while(1){
                            memset(client_newpassword, '\0', sizeof(client_newpassword));
                            if (recv(sd, client_newpassword, sizeof(client_newpassword), 0) < 0){
                                printf("Couldn't receive\n");
                                break;
                            }
                            printf("New password: %s\n", client_newpassword);
                            if(strcmp(client_newpassword, "NULL") == 0){
                                return 0;
                            }
                            if(strcmp(client_newpassword, "bye") == 0){
                                strcpy(server_message, "Goodbye ");
                                strcat(server_message, client_account);
                                if (send(sd, server_message, strlen(server_message), 0) < 0){
                                    printf("Can't send\n");
                                    break;
                                }
                                break;
                            }

                            int i, flag = 1;
                            for (i = 0; client_newpassword[i] != '\0'; i++) {
                                if (!isalpha(client_newpassword[i]) && !isdigit(client_newpassword[i]))
                                    flag = 0;
                            }
                            if(flag == 0){
                                strcpy(server_message, "Error");
                                if (send(sd, server_message, strlen(server_message), 0) < 0){
                                    printf("Can't send\n");
                                    return -1;
                                }
                                continue;
                            }else{
                                changePass(user, client_account, client_newpassword);
                                strcpy(server_message, "Done changed password");
                            }
                            if (send(sd, server_message, strlen(server_message), 0) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            printf("\n%s %s\n", server_message, client_newpassword);
                            if (send(sd, server_message, strlen(server_message), 0) < 0){
                                printf("Can't send\n");
                                return -1;
                            }
                            char encrypted_alpha[100] = "";
                            char encrypted_number[100] = "";
                            for(i = 0; client_newpassword[i] != '\0'; i++){
                                if(isalpha(client_newpassword[i])){
                                    strncat(encrypted_alpha, &client_newpassword[i], 1);
                                }
                                if(isdigit(client_newpassword[i])){
                                    strncat(encrypted_number, &client_newpassword[i], 1);
                                }
                            }
                            printf("\nAlpha: %s", encrypted_alpha);
                            printf("\nNumber: %s\n", encrypted_number);
                            if (strlen(encrypted_alpha) > 0) {
                                if (send(sd, encrypted_alpha, strlen(encrypted_alpha), 0) < 0){
                                    printf("Can't send encrypted_alpha\n");
                                    return -1;
                                }
                            }else{
                                strcpy(encrypted_alpha, "null");
                                if (send(sd, encrypted_alpha, strlen(encrypted_alpha), 0) < 0){
                                    printf("Can't send encrypted_alpha\n");
                                    return -1;
                                }
                            }
                            sleep(1);
                            if (strlen(encrypted_number) > 0) {
                                if (send(sd, encrypted_number, strlen(encrypted_number), 0) < 0){
                                    printf("Can't send encrypted_number\n");
                                    return -1;
                                }
                            }else{
                                strcpy(encrypted_number, "null");
                                if (send(sd, encrypted_number, strlen(encrypted_number), 0) < 0){
                                    printf("Can't send encrypted_number\n");
                                    return -1;
                                }
                            }
                        }
                    }
                }
               clientSockets[i] = 0;
            }
        }
    }

    return 0;
}