#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

void print_menu() {
    printf("\n^### Client Menu ###\n");
    printf("1. Make a donation\n");
    printf("2. Check total donations\n");
    printf("3. View donation history\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}

void print_donation_area_options() {
    printf("\n=== Donation Areas ===\n");
    printf("1. Education\n");
    printf("2. Health\n");
    printf("3. Environment\n");
    printf("4. Animal Welfare\n");
    printf("Enter your choice of area: ");
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char buffer[1024];
    int choice, donation_amount, area_choice;
    char area[1024];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Connection to server failed\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");

    while (1) {
        print_menu();
        scanf("%d", &choice);
        getchar(); 

        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%d", choice);
        send(sock, buffer, strlen(buffer) + 1, 0);

        if (choice == 1) {
            printf("Enter donation amount: ");
            scanf("%d", &donation_amount);
            getchar();  

            print_donation_area_options();
            scanf("%d", &area_choice);
            getchar(); 

            switch (area_choice) {
                case 1:
                    strcpy(area, "Education");
                    break;
                case 2:
                    strcpy(area, "Health");
                    break;
                case 3:
                    strcpy(area, "Environment");
                    break;
                case 4:
                    strcpy(area, "Animal Welfare");
                    break;
                default:
                    strcpy(area, "Unknown");
                    break;
            }

           
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "%d", donation_amount);
            send(sock, buffer, strlen(buffer) + 1, 0);

            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "%s", area);
            send(sock, buffer, strlen(buffer) + 1, 0);

            memset(buffer, 0, sizeof(buffer));
            recv(sock, buffer, sizeof(buffer), 0);
            printf("Server: %s\n", buffer);
        } else if (choice == 2) {
            memset(buffer, 0, sizeof(buffer));
            recv(sock, buffer, sizeof(buffer), 0);
            printf("Server: %s\n", buffer);
        } else if (choice == 3) {
            memset(buffer, 0, sizeof(buffer));
            recv(sock, buffer, sizeof(buffer), 0);
            printf("Server: %s\n", buffer);
        } else if (choice == 4) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid option. Try again.\n");
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
