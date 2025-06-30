#include <winsock2.h>
#include "wintun.h"
#include <Windows.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <ip2string.h>
#include <winternl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <netioapi.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Ws2_32.lib")

static WINTUN_GET_ADAPTER_LUID_FUNC* WintunGetAdapterLUID;
static WINTUN_OPEN_ADAPTER_FUNC*    WinOpenAdapterByName;

int main()
{
    do {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return 1;
        }

        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 1;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8080);

        const char* serverIP = "10.6.7.7";
        if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid server IP address\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connect failed: " << WSAGetLastError() << "\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Connected to server at " << serverIP << ":8080\n";

        std::string message = "Message client";
        char buffer[1024];
        do {

            send(clientSocket, message.c_str(), message.size(), 0);

            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Server response: " << buffer << "\n";
            }
        } while (message != "exit");

        closesocket(clientSocket);
        WSACleanup();
    } while (true);
}

