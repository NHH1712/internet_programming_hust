#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

int findAccount(char* username){
    FILE* file = fopen("nguoidung.txt", "r");
    if (file == NULL) {
        perror("Error opening user file");
        return 0;
    }
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        char u[100];
        sscanf(line, "%s", u);
        if (strcmp(u, username) == 0) {
           // found account
           fclose(file);
           return 1;
        }
    }
    fclose(file);
    return 0;
}
int login(char* username, char* password){
    FILE* file = fopen("nguoidung.txt", "r");
    if (file == NULL) {
        perror("Error opening user file");
        return 0;
    }
    char line[200];
    char new_line[300];
    while (fgets(line, sizeof(line), file)) {
        char u[100], p[100], s[10];
        sscanf(line, "%s %s %s", u, p, s);
        if (strcmp(u, username) == 0) {
            if(strcmp(p, password) == 0){
                if(strcmp(s,"1") == 0){
                    fclose(file);
                    return 3;
                }
                fclose(file);
                return 2;
            }else{
                fclose(file);
                return 1;
            }
        }
    }
    fclose(file);
    return 0;
}
void blockAccount(char* username) {
    FILE* file = fopen("nguoidung.txt", "r");
    if (file == NULL) {
        perror("Error opening user file");
        return;
    }
    char temp_file_name[] = "temp.txt";
    FILE* temp_file = fopen(temp_file_name, "w");
    if (temp_file == NULL) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        char u[100], p[100], s[10];
        sscanf(line, "%s %s %s", u, p, s);
        if (strcmp(u, username) == 0) {
            strcpy(s, "0");
        }
        fprintf(temp_file, "%s %s %s\n", u, p, s);
    }
    fclose(file);
    fclose(temp_file);
    if (remove("nguoidung.txt") != 0) {
        perror("Error deleting user file");
        return;
    }
    if (rename(temp_file_name, "nguoidung.txt") != 0) {
        perror("Error renaming temporary file");
    }
}
void changePassword(char* username, char* newPassword) {
    FILE* file = fopen("nguoidung.txt", "r");
    if (file == NULL) {
        perror("Error opening user file");
        return;
    }

    char temp_file_name[] = "temp.txt";
    FILE* temp_file = fopen(temp_file_name, "w");

    if (temp_file == NULL) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char line[200];
    while (fgets(line, sizeof(line), file)) {
        char u[100], p[100], s[10];
        sscanf(line, "%s %s %s", u, p, s);
        if (strcmp(u, username) == 0) {
            strcpy(p, newPassword);
        }
        fprintf(temp_file, "%s %s %s\n", u, p, s);
    }
    fclose(file);
    fclose(temp_file);
    if (remove("nguoidung.txt") != 0) {
        perror("Error deleting user file");
        return;
    }
    if (rename(temp_file_name, "nguoidung.txt") != 0) {
        perror("Error renaming temporary file");
    }
}

int main(int argc, char *argv[]){
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);

    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    char server_message[200], client_account[100], client_password[100], client_newpassword[100];
    int client_struct_length = sizeof(client_addr);
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_account, '\0', sizeof(client_account));
    memset(client_password, '\0', sizeof(client_password));
    memset(client_newpassword, '\0', sizeof(client_newpassword));
    
    // Create UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");
    
    printf("Listening for incoming messages...\n\n");
    
    // Receive client's message:
    if (recvfrom(socket_desc, client_account, sizeof(client_account), 0,
         (struct sockaddr*)&client_addr, &client_struct_length) < 0){
        printf("Couldn't receive\n");
        return -1;
    }
    if(findAccount(client_account) == 1){
        strcpy(server_message, "Insert password");
    }else{
        strcpy(server_message, "Not found account");
    }
    if (sendto(socket_desc, server_message, strlen(server_message), 0,
         (struct sockaddr*)&client_addr, client_struct_length) < 0){
        printf("Can't send\n");
        return -1;
    }
    if(strcmp(server_message, "Not found account") == 0){
        close(socket_desc);
    }
    strcpy(server_message, " ");
    int count = 0;
    do{
        if(recvfrom(socket_desc, client_password, sizeof(client_password), 0,
            (struct sockaddr*)&client_addr, &client_struct_length) < 0){
            printf("Couldn't receive\n");
            return -1;
        }
        if(login(client_account, client_password) == 3){
            strcpy(server_message, "OK");
        }else if(login(client_account, client_password) == 2){
            strcpy(server_message, "Account not ready");
        }else if(login(client_account, client_password) == 1){
            strcpy(server_message, "Not OK");
        }else{
            strcpy(server_message, "Account is blocked");
        }
        count++;
        if(count >= 3){
            strcpy(server_message, "Account is blocked");
        }
        if (sendto(socket_desc, server_message, strlen(server_message), 0,
            (struct sockaddr*)&client_addr, client_struct_length) < 0){
            printf("Can't send\n");
            return -1;
        }
        if(strcmp(server_message, "Account not ready") == 0){
            close(socket_desc);
        }
    }while(strcmp(server_message, "Not OK") == 0);
    if(count == 3){
        blockAccount(client_account);
        close(socket_desc);
    }
    strcpy(server_message, " ");
    while(1){
        memset(client_newpassword, '\0', sizeof(client_newpassword));
        if (recvfrom(socket_desc, client_newpassword, sizeof(client_newpassword), 0,
            (struct sockaddr*)&client_addr, &client_struct_length) < 0){
            printf("Couldn't receive\n");
            return -1;
        }
        if(strcmp(client_newpassword, "bye") == 0){
            strcpy(server_message, "Goodbye ");
            strcat(server_message, client_account);
            if (sendto(socket_desc, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                printf("Can't send\n");
                return -1;
            }
            return 0;
        }
        changePassword(client_account, client_newpassword);
        int i, is_alnum = 0, is_alpha = 0;
        for (i = 0; client_newpassword[i] != '\0'; i++) {
            if (isalpha(client_newpassword[i]))
                is_alpha = 1;
            if (isalnum(client_newpassword[i]))
                is_alnum = 1;
        }
        if(is_alnum == 0 || is_alpha == 0){
            strcpy(server_message, "Invalid password");
            if (sendto(socket_desc, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                printf("Can't send\n");
                return -1;
            }
        }
        if (sendto(socket_desc, "Done changed password", strlen(server_message), 0,
            (struct sockaddr*)&client_addr, client_struct_length) < 0){
            printf("Can't send\n");
                return -1;
        }
        char encrypted_alpha[100] = "";
        char encrypted_number[100] = "";
        for(i = 0; client_newpassword[i] != '\0'; i++){
            if(isalpha(client_newpassword[i])){
                strncat(encrypted_alpha, &client_newpassword[i], 1);
            }else{
                strncat(encrypted_number, &client_newpassword[i], 1);
            }
        }
        printf("\n%s\n%s\n", encrypted_alpha, encrypted_number);
        // strcpy(server_message, encrypted_alpha);
        if (sendto(socket_desc, encrypted_alpha, strlen(encrypted_alpha), 0,
            (struct sockaddr*)&client_addr, client_struct_length) < 0){
            printf("Can't send\n");
            return -1;
        }
        if (sendto(socket_desc, encrypted_number, strlen(encrypted_number), 0,
            (struct sockaddr*)&client_addr, client_struct_length) < 0){
            printf("Can't send\n");
            return -1;
        }
    }
    // printf("Received message from IP: %s and port: %i\n",
    //        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    // printf("Msg from client: %s %s %s\n", client_account, client_password, client_newpassword);

    // Close the socket:
    close(socket_desc);
}