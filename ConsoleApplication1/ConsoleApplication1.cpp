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
static WINTUN_OPEN_ADAPTER_FUNC* WinOpenAdapterByName;

int main() {
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return 1;
        }

        SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 1;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8080);

        const char* targetIP = "10.6.7.7";
        if (inet_pton(AF_INET, targetIP, &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address/Address not supported: " << targetIP << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }


        if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Server started. Waiting for connections on port 8080...\n";

        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Client connected!\n";

        char buffer[1024];
        int bytesReceived;
        do {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Client says: " << buffer << "\n";

                std::string response = "Server message";
                send(clientSocket, response.c_str(), response.size(), 0);
            }
        } while (true);

        closesocket(clientSocket);
        closesocket(listenSocket);
        WSACleanup();
    } while (true);
    return 0;
}