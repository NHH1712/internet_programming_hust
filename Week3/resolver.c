#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <arpa/inet.h>
#define MAX_USER_LENGTH 50
#define MAX_PASS_LENGTH 50
#define MAX_ACTIVE_CODE_LENGTH 50
#define MAX_STATUS_LENGTH 10
#define MAX_HOMEPAGE_LENGTH 100
typedef struct User {
    char username[MAX_USER_LENGTH];
    char password[MAX_PASS_LENGTH];
    char status[MAX_STATUS_LENGTH];
    char homepage[MAX_HOMEPAGE_LENGTH];
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
        sscanf(line, "%s %s %s %s", user->username, user->password, user->status, user->homepage);
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
        fprintf(file, "%s %s %s %s\n", currentData->username, currentData->password, currentData->status, currentData->homepage);
        currentData = currentData->next;
    }
    fclose(file);
}

int userExist(User* userData, char* username){
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0) {
            return 1; 
        }
        currentData = currentData->next;
    }
    return 0; 
}

void registerUser(User** userData, int* numUsers){
    char username[MAX_USER_LENGTH];
    char password[MAX_PASS_LENGTH];
    char activeCode[MAX_ACTIVE_CODE_LENGTH];
    char homepage[MAX_HOMEPAGE_LENGTH];
    printf("Enter Username: ");
    scanf("%s", username); 
    if (userExist(*userData, username)){
        printf("Account existed \n");
        return;
    }
    printf("Enter Password: ");
    scanf("%s", password);
    printf("Enter Homepage: ");
    scanf("%s", homepage);
    User* user = (User*)malloc(sizeof(User));
    strcpy(user->username, username);
    strcpy(user->password, password);
    strcpy(user->status, "idle");
    strcpy(user->homepage, homepage);
    user->next = NULL;
    if (*userData == NULL){
        *userData = user;
    }else{
        User* current = *userData;
        while (current->next != NULL){
            current = current->next;
        }
        current->next = user;
    }
    *numUsers += 1;
    printf("Successful registration. Activation required.\n");
    writeData(*userData);
}

void activeUser(User* userData, char* username, char *password, char* activeCode){
    User* currentData = userData;
    int count = 0;
    char reActiveCode[MAX_ACTIVE_CODE_LENGTH];
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0 && strcmp(currentData->password, password) == 0){
            if(strcmp(currentData->status, "1") == 0){
                printf("Account is actived\n");
                // return 0;
                return;
            }else{
                if(strcmp(activeCode, "20204963") == 0){
                    strcpy(currentData->status, "1");
                    printf("Account is actived\n");
                    writeData(userData);
                    // return 0;
                    return;
                }
                count++;
                printf("Account is not activated\n");
                while(count != 4){
                    printf("Enter activation code: ");
                    scanf("%s", reActiveCode);
                    if(strcmp(reActiveCode, "20204963") == 0){
                        strcpy(currentData->status, "1");
                        writeData(userData);
                        // return 0;
                        return;
                    }else{
                        count++;
                        printf("Account is not activated\n");
                    }
                }
                strcpy(currentData->status, "0");
                writeData(userData);
                printf("Activation code is incorrect.Account is blocked");
                // return 1;
                return;
            }
        }
        currentData = currentData->next;
    }
    printf("Account is not activated!\n");
    // return 1;
    return;
}
char signedUser[MAX_USER_LENGTH];
char signedPass[MAX_PASS_LENGTH];
bool isSigned = false;
void signIn(User* userData, char* username, char* password){
    User* currentData = userData;
    char repass[MAX_PASS_LENGTH];
    int count = 0;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            strcpy(signedUser, username);
            if(strcmp(currentData->password, password) == 0){
                printf("Hello %s\n", username);
                strcpy(signedPass, password);
                isSigned = true;
                return;
            }else{
                printf("Password is incorrect\n");
                count++;
                while(count != 3){
                    printf("Repeat password: ");
                    scanf("%s", repass);
                    if(strcmp(currentData->password, repass) == 0){
                        printf("Sign in successfully\n");
                        strcpy(signedPass, repass);
                        isSigned = true;
                        return;
                    }else{
                        count++;
                    }   
                }
                strcpy(currentData->status, "idle");
                writeData(userData);
                printf("Password is incorrect. Account is blocked\n");
                // return 1;
                return;
            }
        }
        currentData = currentData->next;
    }
    printf("Cannot find account \n");
    // return 1;
    return;
}

void findByUsername(User* userData, char* username){
    if(isSigned == 0){
        printf("Not sign in! Error search!");
        return;
    }
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            printf("Account is ");
            if(strcmp(currentData->status, "1") == 0){
                printf("active\n");
            }else if(strcmp(currentData->status, "0") == 0){
                printf("blocked\n");
            }else{
                printf("idle\n");
            }
            return ;
        }
        currentData = currentData->next;
    }
    printf("Cannot find account\n");
}

void changePass(User* userData, char* username, char* password, char* newPassword){
    if(isSigned == 0){
        printf("Not sign in! Error change password!");
        return;
    }
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            if(strcmp(currentData->password, password) == 0){
                printf("Password is changed\n");
                strcpy(currentData->password, newPassword);
                writeData(userData);
            }else{
                printf("Current password is incorrect. Please try again\n");
                // return 1;
                return;
            }
        }
        currentData = currentData->next;
    }
    printf("Account not find! Cannot change password!");
    // return 1;
    return;
}

void signOut(User* userData, char* username){
    if(isSigned == 0){
        printf("Not sign in! Error sign out!");
        return;
    }
    // User* currentData = userData;
    // while (currentData != NULL){
    //     if (strcmp(currentData->username, username) == 0){
    //         if (strcmp(currentData->status, "1") == 0){
    //             printf("Goodbye %s\n", username);
    //             // return 0;
    //             return;
    //         }else{
    //             printf("Account is not sign in\n");
    //             // return 1;
    //             return;
    //         }
    //     }
    //     currentData = currentData->next;
    // }
    // printf("Cannot find account\n");
    if(strcmp(username, signedUser) == 0){
        printf("Goodbye %s\n", username);
    }else{
        printf("Cannot find account\n");
    }
    return;
}
void findIP(const char *domain){
    if(isSigned == 0){
        printf("Not sign in! Cannot find IP Address!");
        return;
    }
    struct hostent *host = gethostbyname(domain);
    struct hostent *host_check = gethostbyaddr(domain, sizeof(struct in_addr), AF_INET);
    if (host != NULL) {
        printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));
        printf("Alias IP:\n");
        for (char **ip = host->h_addr_list; *ip != NULL; ip++) {
            printf("%s\n", inet_ntoa(*(struct in_addr *)ip));
        }
    } else {
        printf("Not found information\n");
    }
}
void findDomain(const char *ipaddress){
    if(isSigned == 0){
        printf("Not sign in! Cannot find Domain!");
        return;
    }
    struct in_addr ip;
    if (inet_pton(AF_INET, ipaddress, &ip) != 1) {
        printf("Not found information\n");
        return;
    }
    struct hostent *host = gethostbyaddr(&ip, sizeof(struct in_addr), AF_INET);
    if (host != NULL) {
        printf("Official name: %s\n", host->h_name);
        printf("Alias name:\n");
        for (char **alias = host->h_aliases; *alias != NULL; alias++) {
            printf("%s\n", *alias);
        }
    } else {
        printf("Not found information\n");
    }
}
int main(){
    User* user = NULL;
    int choice;
    int numUsers = 0;
    char findUser[MAX_USER_LENGTH];
    char signOutUser[MAX_USER_LENGTH];
    char changePassUser[MAX_USER_LENGTH];
    char pass[MAX_PASS_LENGTH];
    char newPass[MAX_PASS_LENGTH];
    char userSignIn[MAX_USER_LENGTH];
    char passSignIn[MAX_PASS_LENGTH];
    char activeUsername[MAX_USER_LENGTH];
    char activePassword[MAX_PASS_LENGTH];
    char activeCode[MAX_ACTIVE_CODE_LENGTH];
    readData(&user);
    while(1){
        printf("\nUSER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Activate\n");
        printf("3. Sign in\n");
        printf("4. Search\n");
        printf("5. Change password\n");
        printf("6. Sign out\n");
        printf("7. Homepage with domain name\n");
        printf("8. Homepage with IP address\n");
        printf("Your choice (1-8, other to quit): ");
        // scanf("%d", &choice);
        if(scanf("%d", &choice) != 1){
            printf("\nError Input!\n");
            break;
        }
        switch (choice){
            case 1: 
                registerUser(&user, &numUsers);
                break;
            case 2:
                printf("Enter username to activate: ");scanf("%s", activeUsername); 
                printf("Enter password to activate: ");scanf("%s", activePassword);
                printf("Enter activation code: ");scanf("%s", activeCode);
                getchar(); 
                activeUser(user, activeUsername, activePassword, activeCode);
                break;
            case 3:
                printf("Enter username to sign in: ");
                scanf("%s", userSignIn);
                printf("Enter password to sign in: ");
                scanf("%s", passSignIn);
                signIn(user, userSignIn, passSignIn);
                break;
            case 4:
                printf("Enter username to search: ");
                scanf("%s", findUser);
                findByUsername(user, findUser);
                break;
            case 5:
                printf("Enter username to change password: ");
                scanf("%s", changePassUser); 
                printf("Enter current password: ");
                scanf("%s", pass);
                printf("Enter new password: ");
                scanf("%s", newPass);
                changePass(user, changePassUser, pass, newPass);
                break;         
            case 6:
                printf("Enter username signout: ");
                scanf("%s", signOutUser);
                signOut(user, signOutUser);
                break;
            case 7:
                findIP("google.com");
                break;
            case 8:
                findDomain("google.com");
                break;
            default:
                printf("\nInvalid choice. Please enter a choice between 1 and 8.\n");
                printf("Your choice (1-8, other to quit):");
                scanf("%d", &choice);
                break;
            }
        }
    return 0;
}