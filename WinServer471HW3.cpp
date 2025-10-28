// WinServer471.cpp
// Converted from C to C++ (ChatGPT-5, 2025-10-22)

#include <iostream>
#include <string>
#include <vector>
#include <thread>
// Citation on info: GeekForGeeks
// Mutex is a safeguard with threads to allow singluar
// Usage of resources, a "one at a time" scenario
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

constexpr int SERVER_PORT = 5432;
constexpr int MAX_PENDING = 5;
constexpr int MAX_LINE = 256;

// messageLog stays here so it's globally scooped that's
// Shared amongst threaded users
std::vector<std::string> messageLog;
std::mutex logMutex;

void handleClient(SOCKET cSocket, sockaddr_in clientAddr) {
    std::cout << "Threading successful\n";
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
    std::cout << "Client handler started for: " << ip << '\n';

    // {
    //     std::lock_guard<std::mutex> lg(logMutex);
    //     if(messageLog.empty()) {
    //         const char *msg = "No prior messages.\n";
    //         send
    //     }
    // }

    char buf[MAX_LINE];
    int bytesReceived;

    while ((bytesReceived = recv(cSocket, buf, sizeof(buf), 0)) > 0) {
        std::string message = buf;

        {
            std::lock_guard<std::mutex> lg(logMutex);
            messageLog.push_back(message);
        }

        std::cout << "Client " << ip << " sent: " << message << '\n';

    }

    //std::cout << "Thread started for client " << cIP << " (socket" << cSocket << ")\n";
    //std::cout.flush();

    //while ((bytesReceived = recv(cSocket, buf, sizeof(buf), 0)) > 0) {
    // while (true) {
    //     char ip[INET]
    //     memset(buf, 0, sizeof(buf));
    //     bytesReceived = recv(cSocket, buf, sizeof(buf) - 1, 0);

        // debugging for if bytes are read
        //if (bytesReceived <= 0) {
        //    break;
        //}

        //buf[bytesReceived] = '\0';
        //std::string message = buf;

        //{
            // Copilot
            // This is a recommend usage to ensure safety
            // When it comes to logging messages
            //std::lock_guard<std::mutex> lock(logMutex);
            //messageLog.push_back(message);
        //}

        //std::cout << "Client " << cIP << " sent: " << message << std::endl;
        //std::cout << "[Thread " << std::this_thread::get_id() << "} Client " << clientIP << " sent: " << message << std::endl;
        //messageLog.push_back(message);
    //}

    if (bytesReceived == 0) {
        std::cout << "Client " << ip << " disconnected (graceful).\n";
    } else if (bytesReceived == SOCKET_ERROR) {
        std::cout << "Client " << ip << " recv error: " << WSAGetLastError() << '\n';
    }

    closesocket(cSocket);

    std::cout << "Client " << ip << " disconnected." << "\n\n";
    //closesocket(cSocket);

}

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET s = INVALID_SOCKET
    //new_s = INVALID_SOCKET;
    sockaddr_in sin{};
    //char buf[MAX_LINE];
    int port = SERVER_PORT;
    //std::vector<std::string> messageLog;

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
        SOCKET new_s = accept(s, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        if (new_s == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));

        std::string clientIP = ip;

        std::cout << "Client connected: " << clientIP << std::endl;

        // Citation: Copilot
        // This is a method of multithreading, after an accepted connection has
        // Been established a new thread is made calling a function made above main
        // Allowing each separate user to utilize the function to talk within
        // The server
        sockaddr_in clientCopy = clientAddr;
        std::thread newClient(handleClient, new_s, clientCopy);
        //if (newClient.joinable()) {
        //    std::cout << "Thread created successfully for " << clientIP << std::endl;
        //}
        //std::cout << "Creathed thread (detached) for " << clientIP << " on socket " << new_s << "\n";
        //std::cout.flush();
        newClient.detach();

        std::cout << "Detach successful\n";

        // if (messageLog.empty())
        //     std::cout << "No prior messages to send.\n";
        // else
        //     std::cout << "Sending message log to new client (" << messageLog.size() << " entries)\n";

        // int bytesReceived;
        // while ((bytesReceived = recv(new_s, buf, sizeof(buf), 0)) > 0) {
        //     std::string message(buf);
        //     std::cout << "Client " << ip << " sent: " << message << '\n';
        //     messageLog.push_back(message);
        // }

        // std::cout << "Client " << ip << " disconnected.\n\n";
        // closesocket(new_s);
    }

    file.close();
    closesocket(s);
    WSACleanup();
    return 0;
}
