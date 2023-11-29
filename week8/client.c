#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip_address> <port_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port_number = atoi(argv[2]);
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];
    char server_message[200], client_account[200], client_password[200], client_newpassword[200];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_account, '\0', sizeof(client_account));
    memset(client_password, '\0', sizeof(client_password));
    memset(client_newpassword, '\0', sizeof(client_newpassword));
    // Tạo socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập thông tin địa chỉ của server
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port_number);

    // Chuyển đổi địa chỉ IP từ địa chỉ chuỗi (dạng "127.0.0.1") thành địa chỉ mạng
    if (inet_pton(AF_INET, argv[1], &serverAddress.sin_addr) <= 0) {
        perror("Địa chỉ không hợp lệ hoặc không được hỗ trợ");
        exit(EXIT_FAILURE);
    }

    // Kết nối đến server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Kết nối thất bại");
        exit(EXIT_FAILURE);
    }

    printf("Kết nối thành công đến server.\n");
    // while (1) {
    //     printf("Nhập tin nhắn: ");
    //     fgets(buffer, BUFFER_SIZE, stdin);

    //     // Gửi tin nhắn đến server
    //     if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
    //         perror("Lỗi gửi tin nhắn");
    //         exit(EXIT_FAILURE);
    //     }

    //     // Nhận phản hồi từ server
    //     int valread;
    //     if ((valread = read(clientSocket, buffer, BUFFER_SIZE)) == 0) {
    //         printf("Server đã đóng kết nối.\n");
    //         break;
    //     }

    //     buffer[valread] = '\0';
    //     printf("Server: %s", buffer);
    // }
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
            if(send(clientSocket, client_account, strlen(client_account), 0) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            return 0;
        }
        if(send(clientSocket, client_account, strlen(client_account), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }
        memset(server_message, '\0', sizeof(server_message));
        if(recv(clientSocket, server_message, sizeof(server_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }
        printf("%s\n", server_message);
        if(strcmp(server_message, "Not found account") == 0){
            close(clientSocket);
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
                if(send(clientSocket, client_password, strlen(client_password), 0) < 0){
                    printf("Unable to send message\n");
                    return -1;
                }
                return 0;
            }
            if(send(clientSocket, client_password, strlen(client_password), 0) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            int length = recv(clientSocket, server_message, sizeof(server_message), 0);
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
                    if(send(clientSocket, client_newpassword, strlen(client_newpassword), 0) < 0){
                        printf("Unable to send message\n");
                        return -1;
                    }
                    return 0;
                }
                if(send(clientSocket, client_newpassword, strlen(client_newpassword), 0) < 0){
                    printf("Unable to send message\n");
                    return -1;
                }
                int length = recv(clientSocket, server_message, sizeof(server_message), 0);
                if (length <= 0) {
                    printf("Server disconnected\n");
                    close(clientSocket);
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
                if(recv(clientSocket, server_message, strlen(server_message), 0) < 0){
                    printf("Unable to send message\n");
                    return -1;
                }
                memset(encrypted_alpha, '\0', sizeof(encrypted_alpha));
                int len_alpha = recv(clientSocket, encrypted_alpha, sizeof(encrypted_alpha), 0);
                if (len_alpha > 0) {
                    encrypted_alpha[len_alpha] = 0;
                    if(strcmp(encrypted_alpha, "null") != 0){
                        printf("%s\n", encrypted_alpha);
                    }
                }
                memset(encrypted_number, '\0', sizeof(encrypted_number));
                int len_number = recv(clientSocket, encrypted_number, sizeof(encrypted_number), 0);
                if (len_number > 0) {
                    encrypted_number[len_number] = 0;
                    if(strcmp(encrypted_number, "null") != 0){
                    printf("%s\n", encrypted_number); 
                    } 
                }
            }
        }
    }

    // Đóng kết nối
    close(clientSocket);

    return 0;
}