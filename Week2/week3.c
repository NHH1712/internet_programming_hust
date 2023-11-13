#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

void findIP(const char *domain){
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
struct in_addr ip;
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Error parameter!\n");
        return 1;
    }
    int param1 = atoi(argv[1]);
    char *param2 = argv[2];
    if (param1 == 1) {
        if (inet_pton(AF_INET, param2, &ip) == 1) {
            findDomain(param2);
        } else printf("Wrong parameter \n");
    } else if (param1 == 2) {
        if (param1 == 2) {
            if (!inet_pton(AF_INET, param2, &ip)) {
                findIP(param2);
            } else printf("Wrong parameter \n");
        } else printf("Wrong parameter \n");
    } 
    // else {
    //     printf("Wrong parameter\n");
    // }
    return 0;
}