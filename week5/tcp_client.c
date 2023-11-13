#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        return 1;
    }

    char *client_ip = argv[1];
    int client_port = atoi(argv[2]);

    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[200], client_account[200], client_password[200], client_newpassword[200];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_account, '\0', sizeof(client_account));
    memset(client_password, '\0', sizeof(client_password));
    memset(client_newpassword, '\0', sizeof(client_newpassword));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as the server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client_port);
    server_addr.sin_addr.s_addr = inet_addr(client_ip);

    // Send connection request to the server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with the server successfully\n");
    while(1){
        printf("Enter account: ");
        // scanf("%s", client_account);
        fgets(client_account, sizeof(client_account), stdin);
        size_t lenAcc = strlen(client_account);
        if (lenAcc > 0 && client_account[lenAcc - 1] == '\n') {
            client_account[lenAcc - 1] = '\0';
        }
        if(strcmp(client_account, "") == 0){
            strcpy(client_account, "NULL");
            if(send(socket_desc, client_account, strlen(client_account), 0) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            return 0;
        }
        if(send(socket_desc, client_account, strlen(client_account), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }
        memset(server_message, '\0', sizeof(server_message));
        if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }
        printf("%s\n", server_message);
        if(strcmp(server_message, "Not found account") == 0){
            close(socket_desc);
        }
        
        while(1){
            printf("Enter password: ");
            // scanf("%s", client_password);
            memset(client_password, '\0', sizeof(client_password));
            fgets(client_password, sizeof(client_password), stdin);
            size_t lenPass = strlen(client_password);
            if (lenPass > 0 && client_password[lenPass - 1] == '\n') {
                client_password[lenPass - 1] = '\0';
            }
            if(client_password[0] == '\n'){
                strcpy(client_password, "NULL");
                if(send(socket_desc, client_password, strlen(client_password), 0) < 0){
                    printf("Unable to send message\n");
                    return -1;
                }
                return 0;
            }
            if(send(socket_desc, client_password, strlen(client_password), 0) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            int length = recv(socket_desc, server_message, sizeof(server_message), 0);
            if (length < 0) {
                printf("Error while receiving server's msg\n");
                return -1;
            } else {
                server_message[length] = '\0';
                printf("%s\n", server_message);
                if (strcmp(server_message, "Not OK") != 0) {
                    break; 
                }
            }
        }
        if(strcmp(server_message, "OK") == 0){
            char encrypted_alpha[100];
            char encrypted_number[100];
            while(1){
                printf("Enter new password: ");
                // scanf("%s", client_newpassword);
                memset(client_newpassword, '\0', sizeof(client_newpassword));
                fgets(client_newpassword, sizeof(client_newpassword), stdin);
                size_t lenNewPass = strlen(client_newpassword);
                if (lenNewPass > 0 && client_newpassword[lenNewPass - 1] == '\n') {
                    client_newpassword[lenNewPass - 1] = '\0';
                }
                if(strcmp(client_newpassword, "") == 0){
                    strcpy(client_newpassword, "NULL");
                    if(send(socket_desc, client_newpassword, strlen(client_newpassword), 0) < 0){
                        printf("Unable to send message\n");
                        return -1;
                    }
                    return 0;
                }
                if(send(socket_desc, client_newpassword, strlen(client_newpassword), 0) < 0){
                    printf("Unable to send message\n");
                    return -1;
                }
                int length = recv(socket_desc, server_message, sizeof(server_message), 0);
                if (length <= 0) {
                    printf("Server disconnected\n");
                    close(socket_desc);
                    break;
                }
                server_message[length] = '\0';
                if(strncmp(server_message, "Goodbye", 7) == 0){
                    printf("%s\n", server_message);
                    break;
                }
                if(strcmp(server_message, "Error") == 0){
                    continue;
                }
                if(recv(socket_desc, server_message, strlen(server_message), 0) < 0){
                    printf("Unable to send message\n");
                    return -1;
                }
                memset(encrypted_alpha, '\0', sizeof(encrypted_alpha));
                int len_alpha = recv(socket_desc, encrypted_alpha, sizeof(encrypted_alpha), 0);
                if (len_alpha > 0) {
                    encrypted_alpha[len_alpha] = 0;
                    if(strcmp(encrypted_alpha, "null") != 0){
                        printf("%s\n", encrypted_alpha);
                    }
                }
                memset(encrypted_number, '\0', sizeof(encrypted_number));
                int len_number = recv(socket_desc, encrypted_number, sizeof(encrypted_number), 0);
                if (len_number > 0) {
                    encrypted_number[len_number] = 0;
                    if(strcmp(encrypted_number, "null") != 0){
                    printf("%s\n", encrypted_number); 
                    } 
                }
            }
        }
    }
    // Close the socket:
    close(socket_desc);

    return 0;
}

