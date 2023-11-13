#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USER_LENGTH 50
#define MAX_PASS_LENGTH 50
#define MAX_ACTIVE_CODE_LENGTH 50
#define MAX_STATUS_LENGTH 10

typedef struct User {
    char username[MAX_USER_LENGTH];
    char password[MAX_PASS_LENGTH];
    char status[MAX_STATUS_LENGTH];
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
    printf("Enter Username: ");
    scanf("%s", username); 
    if (userExist(*userData, username)){
        printf("Account existed \n");
        return;
    }
    printf("Enter Password: ");
    scanf("%s", password);
    
    User* user = (User*)malloc(sizeof(User));
    strcpy(user->username, username);
    strcpy(user->password, password);
    strcpy(user->status, "idle");
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
            if(strcmp(currentData->status, "0") == 0){
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
    printf("Account is not activated\n");
    // return 1;
    return;
}

void signIn(User* userData, char* username, char* password){
    User* currentData = userData;
    char repass[MAX_PASS_LENGTH];
    int count = 0;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            if(strcmp(currentData->password, password) == 0){
                printf("Hello %s\n", username);
                // return 0;
                return;
            }else{
                printf("Password is incorrect\n");
                count++;
                while(count != 3){
                    printf("Repeat password: ");
                    scanf("%s", repass);
                    if(strcmp(currentData->password, repass) == 0){
                        printf("Sign in successfully\n");
                        // return 0;
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

    // return 1;
    return;
}

void signOut(User* userData, char* username){
    User* currentData = userData;
    while (currentData != NULL){
        if (strcmp(currentData->username, username) == 0){
            if (strcmp(currentData->status, "1") == 0){
                printf("Goodbye %s\n", username);
                // return 0;
                return;
            }else{
                printf("Account is not sign in\n");
                // return 1;
                return;
            }
        }
        currentData = currentData->next;
    }
    printf("Cannot find account\n");
    // return 1;
    return;
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
        printf("Your choice (1-6, other to quit): ");
        scanf("%d", &choice);
        switch (choice){
            case 1:
                registerUser(&user, &numUsers);
                break;
            case 2:
                printf("Enter username to activate: ");scanf("%s", &activeUsername); 
                printf("Enter password to activate: ");scanf("%s", &activePassword);
                printf("Enter activation code: ");scanf("%s", &activeCode);
                activeUser(user, activeUsername, activePassword, activeCode);
                break;
            case 3:
                printf("Enter username to sign in: ");
                scanf("%s", &userSignIn);
                printf("Enter password to sign in: ");
                scanf("%s", &passSignIn);
                signIn(user, userSignIn, passSignIn);
                break;
            case 4:
                printf("Enter username to search: ");
                scanf("%s", &findUser);
                findByUsername(user, findUser);
                break;
            case 5:
                printf("Enter username to change password: ");
                scanf("%s", &changePassUser); 
                printf("Enter current password: ");
                scanf("%s", &pass);
                printf("Enter new password: ");
                scanf("%s", &newPass);
                changePass(user, changePassUser, pass, newPass);
                break;         
            case 6:
                printf("Enter username signout: ");
                scanf("%s", &signOutUser);
                signOut(user, signOutUser);
                break;
            default:
                printf("Invalid choice. Please enter a choice between 1 and 6.\n");
                printf("Your choice (1-6, other to quit):");
                scanf("%d", &choice);
                break;
            }
        }
    return 0;
}