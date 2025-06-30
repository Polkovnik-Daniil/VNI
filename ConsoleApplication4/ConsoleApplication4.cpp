#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

SOCKET sock;
WSADATA wsaData;

static DWORD WINAPI
ReceivePackets(_Inout_ DWORD_PTR SessionPtr) {
    
    while (true) {
        char buffer[BUFFER_SIZE];

        sockaddr_in targetAddr;
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(8080);
        inet_pton(AF_INET, "10.211.55.4", &targetAddr.sin_addr);

        int senderLen = sizeof(targetAddr);
        
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&targetAddr, &senderLen);
        if (bytes > 0) {
            buffer[bytes] = 0;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &targetAddr.sin_addr, ip, INET_ADDRSTRLEN);
            std::cout << "Received message from " << ip << ": " << buffer << std::endl;
        }
    }

}

static DWORD WINAPI
SendPackets(_Inout_ DWORD_PTR SessionPtr) {
    
    sockaddr_in targetAddr;
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "10.211.55.4", &targetAddr.sin_addr);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        const char* message = "Hello from specific interface!";
        if (sendto(::sock, message, strlen(message), 0,
            (sockaddr*)&targetAddr, sizeof(targetAddr)) == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << "\n";
        }
        else {
            std::cout << "Message sent successfully\n";
        }
    }
}

int main() {
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    if (WSAStartup(MAKEWORD(2, 2), &::wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    ::sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (::sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        return 1;
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "10.6.7.7", &localAddr.sin_addr);

    if (bind(::sock, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        return 1;
    }

    HANDLE Workers[] = { CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceivePackets, NULL, 0, NULL),
                         CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendPackets, NULL, 0, NULL) };
    if (!Workers[0] || !Workers[1])
    {
        return 0;
    }
    WaitForMultipleObjectsEx(_countof(Workers), Workers, TRUE, INFINITE, TRUE);
    closesocket(::sock);
    WSACleanup();
    return 0;
}