#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    char region[50];
    char disaster_type[50];
    float severity;
} Disaster;

Disaster disaster_db[100];
int disaster_count = 0;

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int recv_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (recv_size <= 0) {
            printf("Client disconnected.\n");
            closesocket(client_socket);
            return;
        }

        buffer[recv_size] = '\0';
        printf("Received: %s\n", buffer);

        if (strncmp(buffer, "AFET_BÝLDÝR", 11) == 0) {
           
            char region[50], disaster_type[50];
            float severity;
            sscanf(buffer + 12, "%s %s %f", region, disaster_type, &severity);

            strcpy(disaster_db[disaster_count].region, region);
            strcpy(disaster_db[disaster_count].disaster_type, disaster_type);
            disaster_db[disaster_count].severity = severity;
            disaster_count++;

            sprintf(response, "Afet kaydedildi: %s %s %.1f\n", region, disaster_type, severity);
        } else if (strncmp(buffer, "DURUM_SOR", 9) == 0) {
            
            char region[50];
            sscanf(buffer + 10, "%s", region);

            int found = 0;
            int i;
            memset(response, 0, BUFFER_SIZE);
            for ( i = 0; i < disaster_count; i++) {
                if (strcmp(disaster_db[i].region, region) == 0) {
                    sprintf(response + strlen(response), "Bolge: %s, Afet: %s, Siddet: %.1f\n",
                            disaster_db[i].region, disaster_db[i].disaster_type, disaster_db[i].severity);
                    found = 1;
                }
        }
            if (!found) {
                sprintf(response, "Bolge: %s icin kayitli afet bulunamadi.\n", region);
            }
        } else {
            sprintf(response, "Bilinmeyen komut.\n");
        }

        send(client_socket, response, strlen(response), 0);
    }
}
int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WinSock baslatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket olusturulamadi. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind basarisiz. Hata kodu: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    listen(server_socket, 3);
    printf("Server dinleniyor...\n");

    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) != INVALID_SOCKET) {
        printf("Yeni bir client baglandi.\n");
        handle_client(client_socket);
    }

    if (client_socket == INVALID_SOCKET) {
        printf("Accept basarisiz. Hata kodu: %d\n", WSAGetLastError());
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

