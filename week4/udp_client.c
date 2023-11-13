#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    if (argc != 3) {
        printf("Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        return 1;
    }
    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[100], client_account[100], client_password[100], client_newpassword[100];
    int server_struct_length = sizeof(server_addr);
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_account, '\0', sizeof(client_account));
    memset(client_password, '\0', sizeof(client_password));
    memset(client_newpassword, '\0', sizeof(client_newpassword));
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    
    // Get input from the user:
    printf("Enter account: ");
    scanf("%s", client_account);
    // Send the message to server:
    if(sendto(socket_desc, client_account, strlen(client_account), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    // Receive the server's response:
    if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
         (struct sockaddr*)&server_addr, &server_struct_length) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    printf("%s\n", server_message);
    if(strcmp(server_message, "Not found account") == 0){
        return 0;
    }
    if(strcmp(server_message, "Insert password") == 0){
        while(1){
            printf("Enter password: ");
            scanf("%s", client_password);
            if(sendto(socket_desc, client_password, strlen(client_password), 0,
                (struct sockaddr*)&server_addr, server_struct_length) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            // memset(server_message, '\0', sizeof(server_message));
            int length = recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                (struct sockaddr*)&server_addr, &server_struct_length);
            if(length > 0){
                server_message[length] = 0;
            }
            printf("%s\n", server_message);
            if(strcmp(server_message, "OK") == 0){
                break;
            }
            if(strcmp(server_message, "Not OK") != 0){
                return 0;
            }
        }
    }
    while(1){
        printf("Enter new password: ");
        scanf("%s", client_newpassword);
        if(sendto(socket_desc, client_newpassword, strlen(client_newpassword), 0,
            (struct sockaddr*)&server_addr, server_struct_length) < 0){
            printf("Unable to send message\n");
            return -1;
        }
        int length = recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                (struct sockaddr*)&server_addr, &server_struct_length);
        if(length > 0){
            server_message[length] = 0;
        }
        if(strncmp(server_message, "Goodbye", 7) == 0){
            printf("\n%s\n", server_message);
            return 0;
        }
        char encrypted_alpha[200], encrypted_number[200];
        int len_alpha = recvfrom(socket_desc, encrypted_alpha, sizeof(encrypted_alpha), 0,
                (struct sockaddr*)&server_addr, &server_struct_length);
        int len_number = recvfrom(socket_desc, encrypted_number, sizeof(encrypted_number), 0,
                (struct sockaddr*)&server_addr, &server_struct_length);
        if(len_alpha > 0){
            encrypted_alpha[len_alpha] = 0;
        }
        if(len_number > 0){
            encrypted_number[len_number] = 0;
        }
        printf("%s\n%s\n", encrypted_alpha, encrypted_number);
    } 
    // Close the socket:
    close(socket_desc);
    return 0;
}
