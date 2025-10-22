// WinServer471.cpp
// Converted from C to C++ (ChatGPT-5, 2025-10-22)

#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

constexpr int SERVER_PORT = 5432;
constexpr int MAX_PENDING = 5;
constexpr int MAX_LINE = 256;

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET s = INVALID_SOCKET, new_s = INVALID_SOCKET;
    sockaddr_in sin{};
    char buf[MAX_LINE];
    int port = SERVER_PORT;
    std::vector<std::string> messageLog;

    if (argc == 2)
        port = std::stoi(argv[1]);
    else {
        std::cerr << "Usage: WinServer471 <port>\n";
        return 1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    if (bind(s, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    listen(s, MAX_PENDING);
    std::cout << "Server listening on port " << port << "...\n";

    while (true) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        new_s = accept(s, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        if (new_s == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
        std::cout << "Client connected: " << ip << "\n";

        if (messageLog.empty())
            std::cout << "No prior messages to send.\n";
        else
            std::cout << "Sending message log to new client (" << messageLog.size() << " entries)\n";

        int bytesReceived;
        while ((bytesReceived = recv(new_s, buf, sizeof(buf), 0)) > 0) {
            std::string message(buf);
            std::cout << "Client " << ip << " sent: " << message << '\n';
            messageLog.push_back(message);
        }

        std::cout << "Client " << ip << " disconnected.\n\n";
        closesocket(new_s);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
