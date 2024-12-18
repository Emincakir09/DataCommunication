#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 8080
#define MAX_HISTORY 100

WSADATA wsaData;
SOCKET server_sock;
struct sockaddr_in server_addr, client_addr;
char donation_history[MAX_HISTORY][1024];
int donation_history_count = 0;
int total_donations = 0;
CRITICAL_SECTION cs; 

void update_donation_history(int donation_amount, const char* area) {
    EnterCriticalSection(&cs); 
    if (donation_history_count < MAX_HISTORY) {
        sprintf(donation_history[donation_history_count], "Donation: %d TL to %s", donation_amount, area);
        donation_history_count++;
    }
    else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(donation_history[i], donation_history[i + 1]);
        }
        sprintf(donation_history[MAX_HISTORY - 1], "Donation: %d TL to %s", donation_amount, area);
    }
    LeaveCriticalSection(&cs); 
}

void process_donation(int donation_amount, const char* area, SOCKET client_sock) {
    char response[1024];

    if (donation_amount > 0) {
        EnterCriticalSection(&cs);
        total_donations += donation_amount;
        update_donation_history(donation_amount, area);
        LeaveCriticalSection(&cs);

        sprintf(response, "Donation successful! Total donations: %d", total_donations);
        printf("Donation of %d TL made to %s. Total donations: %d\n", donation_amount, area, total_donations);
    }
    else {
        strcpy(response, "Invalid donation amount. Please enter a positive value.");
    }

    send(client_sock, response, strlen(response) + 1, 0);
}

void show_donation_history(SOCKET client_sock) {
    char response[1024] = "Donation History:\n";
    EnterCriticalSection(&cs);
    for (int i = 0; i < donation_history_count; i++) {
        strcat(response, donation_history[i]);
        strcat(response, "\n");
    }
    LeaveCriticalSection(&cs);
    send(client_sock, response, strlen(response) + 1, 0);
}

DWORD WINAPI client_handler(LPVOID client_socket) {
    SOCKET client_sock = *(SOCKET*)client_socket;
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        recv(client_sock, buffer, sizeof(buffer), 0);
        int choice = atoi(buffer);

        if (choice == 1) {
            recv(client_sock, buffer, sizeof(buffer), 0);
            int donation_amount = atoi(buffer);

            memset(buffer, 0, sizeof(buffer));
            recv(client_sock, buffer, sizeof(buffer), 0);
            char area[1024];
            strcpy(area, buffer);

            process_donation(donation_amount, area, client_sock);
        }
        else if (choice == 2) {
            sprintf(buffer, "Total donations : %d", total_donations);
            send(client_sock, buffer, strlen(buffer) + 1, 0);
        }
        else if (choice == 3) {
            show_donation_history(client_sock);
        }
        else if (choice == 4) {
            printf("Client disconnected\n");
            break;
        }
        else {
            strcpy(buffer, "Invalid option.");
            send(client_sock, buffer, strlen(buffer) + 1, 0);
        }
    }

    closesocket(client_sock);
    return 0;
}

int main() {
    InitializeCriticalSection(&cs);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Binding failed\n");
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    if (listen(server_sock, 3) == SOCKET_ERROR) {
        printf("Listening failed\n");
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d\n", PORT);

    int client_len = sizeof(client_addr);
    SOCKET client_sock;

    while ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len)) != INVALID_SOCKET) {
        printf("Client connected\n");
        HANDLE thread = CreateThread(NULL, 0, client_handler, &client_sock, 0, NULL);
        if (thread == NULL) {
            printf("Thread creation failed\n");
            closesocket(client_sock);
        }
    }

    DeleteCriticalSection(&cs);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}
