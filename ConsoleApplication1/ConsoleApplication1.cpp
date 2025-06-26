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
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    SOCKET clientSocket;
    SOCKET serverSocket;
    int bytesReceived;

    do {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
            return 1;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;

        const char* bindIP = "10.6.7.7";
        if (inet_pton(AF_INET, bindIP, &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid IP address\n";
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        serverAddr.sin_port = htons(8080);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Server started. Waiting for connections..." << std::endl;

        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }


        char buffer[1024];
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
        }

        std::cout << "Message:\t" << buffer << std::endl;


        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        //////////////////////////////////////////////////////////////////////////////////

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
            return 1;
        }

        clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8888);

        const char* targetIP = "10.6.7.7";
        if (inet_pton(AF_INET, targetIP, &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address/Address not supported: " << targetIP << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        std::string message = "Hi";

        int sendResult = sendto(
            clientSocket,
            message.c_str(),
            message.size(),
            0,
            (sockaddr*)&serverAddr,
            sizeof(serverAddr)
        );

        if (sendResult == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            break;
        }

        std::cout << "Sent " << sendResult << " bytes to " << targetIP << std::endl;

        closesocket(clientSocket);
        WSACleanup();

    } while (true);


    return 0;
}