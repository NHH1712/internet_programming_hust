#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct User {
    char username[50];
    char password[50];
    char status[50];
    struct User* next;
} User;

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }
    User* user = NULL;
    readData(&user);
    int server_port = atoi(argv[1]);

    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[200];
    char client_account[200], client_password[200], client_newpassword[200];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_account, '\0', sizeof(client_account));
    memset(client_password, '\0', sizeof(client_password));
    memset(client_newpassword, '\0', sizeof(client_newpassword));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        return -1;
    }
    printf("Listening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

    if (client_sock < 0) {
        printf("Can't accept\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    while(1){
        if (recv(client_sock, client_account, sizeof(client_account), 0) < 0){
            printf("Couldn't receive\n");
            return -1;
        }
        printf("Account: %s\n", client_account);
        if(strcmp(client_account, "NULL") == 0){
            return 0;
        }
        if(findAccount(user, client_account) == 1){
            strcpy(server_message, "Insert password");
            if (send(client_sock, server_message, strlen(server_message), 0) < 0){
                printf("Can't send\n");
                return -1;
            }
        }else{
            strcpy(server_message, "Not found account");
            if (send(client_sock, server_message, strlen(server_message), 0) < 0){
                printf("Can't send\n");
                return -1;
            }
            // close(client_sock);
            // close(socket_desc);
        }
        int count = 0;
        int login_successful = 0;
        while(!login_successful && count < 3){
            memset(client_password, '\0', sizeof(client_password));
            if(recv(client_sock, client_password, sizeof(client_password), 0) < 0){
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
            if (send(client_sock, server_message, strlen(server_message), 0)< 0){
                printf("Can't send\n");
                return -1;
            }
        }
        if (count >= 3) {
            close(client_sock);
        }
        if (login_successful) {
            while(1){
                memset(client_newpassword, '\0', sizeof(client_newpassword));
                if (recv(client_sock, client_newpassword, sizeof(client_newpassword), 0) < 0){
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
                    if (send(client_sock, server_message, strlen(server_message), 0) < 0){
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
                    if (send(client_sock, server_message, strlen(server_message), 0) < 0){
                        printf("Can't send\n");
                        return -1;
                    }
                    continue;
                }else{
                    changePass(user, client_account, client_newpassword);
                    strcpy(server_message, "Done changed password");
                }
                if (send(client_sock, server_message, strlen(server_message), 0) < 0){
                    printf("Can't send\n");
                    return -1;
                }
                printf("\n%s %s\n", server_message, client_newpassword);
                if (send(client_sock, server_message, strlen(server_message), 0) < 0){
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
                    if (send(client_sock, encrypted_alpha, strlen(encrypted_alpha), 0) < 0){
                        printf("Can't send encrypted_alpha\n");
                        return -1;
                    }
                }else{
                    strcpy(encrypted_alpha, "null");
                    if (send(client_sock, encrypted_alpha, strlen(encrypted_alpha), 0) < 0){
                        printf("Can't send encrypted_alpha\n");
                        return -1;
                    }
                }
                sleep(1);
                if (strlen(encrypted_number) > 0) {
                    if (send(client_sock, encrypted_number, strlen(encrypted_number), 0) < 0){
                        printf("Can't send encrypted_number\n");
                        return -1;
                    }
                }else{
                    strcpy(encrypted_number, "null");
                    if (send(client_sock, encrypted_number, strlen(encrypted_number), 0) < 0){
                        printf("Can't send encrypted_number\n");
                        return -1;
                    }
                }
            }
        }
    }
    // Close the client socket and the server socket:
    close(client_sock);
    close(socket_desc);

    return 0;
}

