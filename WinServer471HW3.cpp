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
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

constexpr int SERVER_PORT = 5432;
constexpr int MAX_PENDING = 5;
constexpr int MAX_LINE = 256;

// messageLog stays here so it's globally scooped that's
// Shared amongst threaded users
std::vector<std::string> messageLog;
std::mutex logMutex;
std::vector<SOCKET> clients;  // stores list of connected sockets
std::mutex clientsMutex; // Prevents race condition between clients, one thread adds, another broadcasts, a third removes clients.

void handleClient(SOCKET cSocket, sockaddr_in clientAddr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));

    // Pulled and repurposed from https://stackoverflow.com/questions/58052740/how-can-i-print-the-value-of-a-port-number && 
    // https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/    
    
    int port = ntohs(clientAddr.sin_port); 


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

        std::string fullMsg = "Client " + std::string(ip) + ":" + std::to_string(port) + " sent: " + message;
        std::cout << fullMsg << '\n';

        {
            //mutual exclusion to prevent all clients from trying to send everything to server at the same time,
            // taken from and repurposed, https://en.cppreference.com/w/cpp/thread/lock_guard.html
            
            std::lock_guard<std::mutex> lockname(clientsMutex); 


            for (SOCKET other : clients) { // standard for loop using vector of clients to ensure message is sent to all clients except the sender
                if (other != cSocket) {
                    send(other, fullMsg.c_str(), static_cast<int>(fullMsg.size()) + 1, 0);
                }
            }
        }
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


    
    std::lock_guard<std::mutex> lockname(clientsMutex);

    //CITATION: ChatGPT for better understanding, https://www.geeksforgeeks.org/cpp/erase-remove-idiom-in-cpp/, https://cplusplus.com/reference/vector/vector/erase/
    clients.erase(std::remove(clients.begin(), clients.end(), cSocket), clients.end()); 
    
    
    closesocket(cSocket);

    std::cout << "\nThe client at " + std::string(ip) + ":" + std::to_string(port) + " has disconnected from the server\n\n";
    //closesocket(cSocket);

}

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET s = INVALID_SOCKET;
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
    std::cout << "Server is at address: 192.168.56.1\n";
    std::cout << "Server is using port " << port << "...\n";
    

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
        int clientPort = ntohs(clientAddr.sin_port);
        std::string clientIP = ip;

        std::cout << "\nThe client at " << clientIP << ":" << clientPort << " has connected to the server\n";

        if (messageLog.empty()) {
            std::cout << "The server has no message log to send to this client\n\n";
        }
        else {
            std::cout << "The server sent the message log to this client\n\n";
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            if (clients.empty()) {
                std::string noclient = "No other clients connected to the server.\n";
                send(new_s, noclient.c_str(), static_cast<int>(noclient.size()) + 1, 0);
            } else {
                std::string yesclient = "The following client(s) are connected to the server:\n";
                send(new_s, yesclient.c_str(), static_cast<int>(yesclient.size()) + 1, 0);

                for (SOCKET other : clients) {
                    if (other != new_s) {
                        sockaddr_in addr;
                        int len = sizeof(addr);
                        getpeername(other, (sockaddr*)&addr, &len); // Citation, https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-getpeername, + GPT for understand of how to print
                        // a corresponding client address
                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
                        int port = ntohs(addr.sin_port);

                        std::string info = "- " + std::string(ip) + ":" + std::to_string(port) + "\n";
                        send(new_s, info.c_str(), static_cast<int>(info.size()) + 1, 0);
                    }
                }
            }

            std::string join = "A new client has joined: " + clientIP + ":" + std::to_string(clientPort) + "\n";
            for (SOCKET other : clients) {
                if (other != new_s) {
                    send(other, join.c_str(), static_cast<int>(join.size()) + 1, 0);
                }
            }
            clients.push_back(new_s);
        }
    
        sockaddr_in clientCopy = clientAddr;
        std::thread newClient(handleClient, new_s, clientCopy);
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back(new_s);

        // Citation: Copilot
        // This is a method of multithreading, after an accepted connection has
        // Been established a new thread is made calling a function made above main
        // Allowing each separate user to utilize the function to talk within
        // The server
        //if (newClient.joinable()) {
        //    std::cout << "Thread created successfully for " << clientIP << std::endl;
        //}
        //std::cout << "Creathed thread (detached) for " << clientIP << " on socket " << new_s << "\n";
        //std::cout.flush();
        newClient.detach();

//            int bytesReceived;
//        while ((bytesReceived = recv(new_s, buf, sizeof(buf), 0)) > 0) {
//            std::string message(buf);
//            std::cout << "Client " << ip << " sent: " << message << '\n';
//            messageLog.push_back(message);
//        }

        // std::cout << "Client " << ip << " disconnected.\n\n";
        // closesocket(new_s);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
