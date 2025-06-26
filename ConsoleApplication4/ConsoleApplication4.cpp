#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

int main() {

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    while (true) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return 1;
        }

        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 1;
        }

        sockaddr_in localAddr;
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(0);
        inet_pton(AF_INET, "10.6.7.7", &localAddr.sin_addr);

        if (bind(sock, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        sockaddr_in targetAddr;
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(12345);
        inet_pton(AF_INET, "10.211.55.4", &targetAddr.sin_addr);

        const char* message = "Hello from specific interface!";
        if (sendto(sock, message, strlen(message), 0,
            (sockaddr*)&targetAddr, sizeof(targetAddr)) == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << "\n";
        }
        else {
            std::cout << "Message sent successfully\n";
        }

        closesocket(sock);
        WSACleanup();
    }
    return 0;
}