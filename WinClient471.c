// Code adapted from section 1.4.2 of "Computer Networks: A Systems Approach",
//   version 6.02dev, by Larry L Peterson and Bruce S. Davie
//   https://book.systemsapproach.org/foundation/software.html

#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <errno.h>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT 5432
#define MAX_LINE 256

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET s = INVALID_SOCKET;
    struct sockaddr_in sin;
    char* host;
    char buf[MAX_LINE];
    int len;
    int port = SERVER_PORT;

    char input[MAX_LINE];
    int result;
    size_t length;
    char line[MAX_LINE];

    // Citation:
    // Model: ChatGPT-5, Date: 9/23/2025
    // Has file be safely opened with _fsopen
    // _SH_DENYNO was meant to allow access to the textfile even when
    // Accessed by WinClient
    FILE* file = _fsopen("WinClient471.txt", "a+", _SH_DENYNO);
    if (file == NULL) {
        perror("Error, WinClient471 text file doesn't exist\n");
        return 1;
    }


    if (argc==2) {
        host = argv[1];
    }
    else if (argc==3) {
        host = argv[1];
        port = atoi(argv[2]);
    }
    else {
        fprintf(stderr, "usage: simplex-talk host\n");
        exit(1);
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    /* build address data structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, host, &sin.sin_addr.s_addr);
    sin.sin_port = htons(port);

     /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("simplex-talk: socket");
        WSACleanup();
        exit(1);
    }
    if (connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        closesocket(s);
        WSACleanup();
        exit(1);
    }
    
    printf("Client is connected to the server.\n");

    // Citations: 
    // Model: ChatGPT-5, Date: 9/23/2025
    // fseek tried to look through the file at the end
    // ftell is meant to check if there is any content from the file
    // If no content is in the file then returns 0
    // Rewird() is used to set the point back to the beginning
    // Since fseek is used to check if file is empty
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {

        printf("No prior message log from the server.\n");

    } else {

        printf("Client received message log:");
        rewind(file);
        while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);

        }
    }

    /* main loop: get and send lines of text */
    //while (fgets(buf, sizeof(buf), stdin)) {

    while (1) {

        printf("Enter message to send to server: ");

        if (fgets(buf, sizeof(buf), stdin)) {

            // Citation needed (beginning)

            // Citation:
            // Model: ChatGPT-5, Date: 9/23/2025
            // strcspn is meant to remove the newline from the buffer
            // This is so strcmp can work properly when comparing
            // "quit" to buffer
            // strcpy_s is a safe alternative to strcpy that allows input
            // To hold the modified non-newline version of the buffer
            buf[strcspn(buf, "\r\n")] = '\0';
            printf("Client sending message: \"%s\"\n", buf);
            strcpy_s(input, sizeof (input), buf);


        }

        result = strcmp(input, "quit");
        if (result == 0) {
            break;
        }

        fprintf(file, "%s: %s\n", host, buf);
    
        // Citation needed (beginning)

        // Citation:
        // Model: ChatGPT-5, Date: 9/23/2025
        // Takes in the legnth of the buffer message
        // first line inside if is to add back the newline removed previously
        // second line inside if is to "terminate" the buf
        // All of this to revert the changes made by strcspn
        length = strlen(buf);
        if (length + 1 < sizeof(buf)) {
            buf[length] = '\n';
            buf[length + 1] = '\0';
        }

        buf[MAX_LINE - 1] = '\0';
        len = strlen(buf) + 1;
        send(s, buf, len, 0);
    }

    fclose(file);
    printf("Client is disconnected from the server.");
    closesocket(s);
    WSACleanup();
    return 0;
    }
