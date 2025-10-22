// Code adapted from section 1.4.2 of "Computer Networks: A Systems Approach",
//   version 6.02dev, by Larry L Peterson and Bruce S. Davie
//   https://book.systemsapproach.org/foundation/software.html

#include <stdio.h>
#include <share.h>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT  5432
#define MAX_PENDING  5
#define MAX_LINE     256

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET s = INVALID_SOCKET, new_s = INVALID_SOCKET;
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int buf_len, addr_len = sizeof(struct sockaddr_in), port = SERVER_PORT;

    char hostname[256];

    // IEEE Citation:
    // GeeksforGeeks, “Array of Strings in C,” GeeksforGeeks, Oct. 08, 2022. https://www.geeksforgeeks.org/c/array-of-strings-in-c/
    // Creates char array meant for message strings with an index number
    char *messagearray[MAX_LINE];
    int index = 0;

    // Citation: 
    // Model: ChatGPT-5, Date: 9/23/2025
    // Uses the internet address' string lenght
    char ip[INET_ADDRSTRLEN];

    printf("Server is at address: 192.168.4.25\n");
    printf("Server is using port: %d\n\n", port);

    if (argc == 2) {
        port = atoi(argv[1]);
    } else {
        fprintf(stderr, "Server.exe requires a port");
        exit(1);
    }
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("Error with hostname");
        return 1;
    }

    /* build address data structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    /* setup passive open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("simplex-talk: socket");
        WSACleanup();
        exit(1);
    }
    if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) == SOCKET_ERROR) {
        perror("simplex-talk: bind");
        closesocket(s);
        WSACleanup();
        exit(1);
    }
    listen(s, MAX_PENDING);

     /* wait for connection, then receive and print text */
    while(1) {
        if ((new_s = accept(s, (struct sockaddr*)&sin, &addr_len)) == INVALID_SOCKET) {
            perror("simplex-talk: accept");
            closesocket(s);
            WSACleanup();
            exit(1);
        }

        // Citation:
        // Model: ChatGPT-5, Date: 9/23/2025
        // This converts the internet's network to become human
        // Readable for uses that are going to be needed for this program
        inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof ip);

        // Citation:
        // Model: ChatGPT-5, Date: 9/23/2025
        // This converts the internet's network structure to become human
        // Readable for uses that are going to be needed for this program
        printf("The client at %s has connected to the server\n", ip);

        if (index == 0) {
            printf("The server has no message log to send to this client\n");
        } else {
            printf("The server sent the message log to this client\n");
        }


        while (buf_len = recv(new_s, buf, sizeof(buf), 0)) {
            printf("Client %s sent: %s", ip, buf);
        
            // Citation:
            // GeeksforGeeks, “C Program to Insert an Element in an Array,” 
            // GeeksforGeeks, Aug. 30, 2019. https://www.geeksforgeeks.org/c/c-program-to-insert-an-element-in-an-array/ (accessed Sep. 25, 2025).
            // Meant to add the message to an array and increase it's index for the next
            // Message
            messagearray[index] = buf;
            index++;
        }
        printf("The client at %s has disconnected from the server\n\n", ip);
        closesocket(new_s);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
