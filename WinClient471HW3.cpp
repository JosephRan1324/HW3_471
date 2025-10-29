// WinClient471.cpp
// Final working version (ChatGPT-5, 2025-10-22)
// Converts C version to modern C++ with proper logging and output formatting

#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
std::mutex coutMutex;
constexpr int SERVER_PORT = 5432;
constexpr int MAX_LINE = 256;

// Citations:
// https://pubs.opengroup.org/onlinepubs/007904975/functions/recv.html - For learning recv function
// https://www.geeksforgeeks.org/c/tcp-server-client-implementation-in-c/ - Similar usage of void func(int sockfd) for TCP implementation
// https://learn.microsoft.com/en-us/windows/win32/winsock/complete-client-code Similar use of recv function
// Gpt for better understanding of how each of the codes in this link works.

void recvThread(SOCKET clientsend) {
    char tempdata[MAX_LINE];
    int n;
    while ((n = recv(clientsend, tempdata, sizeof(tempdata) - 1, 0)) > 0) {

        tempdata[n] = '\0';
        std::string msg(tempdata);
        std::cout << "\n" << tempdata << std::endl;
        std::cout << "Enter message to send to server: ";
    }

    if (n == 0)
        std::cout << "\nServer disconnected.\n";
    else
        std::cout << "\nConnection error: " << WSAGetLastError() << "\n";
}

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET s = INVALID_SOCKET;
    sockaddr_in sin{};
    std::string host;
    int port = SERVER_PORT;

    if (argc == 2) {
        host = argv[1];
    } else if (argc == 3) {
        host = argv[1];
        port = std::stoi(argv[2]);
    } else {
        std::cerr << "Usage: WinClient471 <host> [port]\n";
        return 1;
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Build address
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, host.c_str(), &sin.sin_addr.s_addr);
    sin.sin_port = htons(port);

    // Create socket
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // Connect
    if (connect(s, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) < 0) {
        std::cerr << "Connection failed\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected to server.\n";
    std::thread listener(recvThread, s);
    listener.detach();
    
    // Open (or create) log file for both reading and appending
    std::ifstream infile("MessageLog.txt");
    if (!infile) {
        std::cout << "No prior message log from the server.\n";
    } else {
        infile.seekg(0, std::ios::end);
        if (infile.tellg() == 0) {
            std::cout << "No prior message log from the server.\n";
        } else {
            std::cout << "Client message log:\n";
            infile.seekg(0, std::ios::beg);
            std::string line;
            while (std::getline(infile, line))
                std::cout << line << '\n';
        }
        infile.close();
    }

    // Open file for appending new entries
    std::ofstream file("MessageLog.txt", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open MessageLog.txt\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    std::string input;
    char buf[MAX_LINE];

    // Main message-sending loop
    while (true) {
        std::cout << "Enter message to send to server: ";
        std::getline(std::cin, input);

        if (input == "quit") break;

        // Log locally
        if (!input.empty()) {
            file << host << ": " << input << '\n';
            file.flush(); // Ensure message is written immediately
        }

        // Print confirmation
        std::cout << "Client sending message: \"" << input << "\"\n";

        // Copy to buffer safely
        strncpy_s(buf, input.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        int len = static_cast<int>(strlen(buf)) + 1;
        send(s, buf, len, 0);
    }

    std::cout << "Client disconnected.\n";

    file.close();
    closesocket(s);
    WSACleanup();
    return 0;
}
