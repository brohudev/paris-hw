#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    const char *message = "Hello from client";

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // Sending message to server
    send(sock, message, strlen(message), 0);
    std::cout << "Message sent to server." << std::endl;

    // Receiving acknowledgment from server
    valread = read(sock, buffer, BUFFER_SIZE);
    std::cout << "Server: " << buffer << std::endl;

    close(sock);
    return 0;
}

/*
1. Initialize socket for client
2. Connect to server
3. Loop indefinitely:
   4. Prompt user for input
   5. Parse user input
   6. If input is "get filename":
       7. Send message to server requesting file
       8. Receive response from server
       9. If file exists:
             10. Open file for writing
             11. Receive file contents from server
             12. Write received contents to file
             13. Print message indicating file received and its size
          14. Else:
             15. Print message indicating file is missing
   16. Else if input is "exit":
       17. Send "exit" message to server
       18. Close connection to server
       19. Terminate client
   20. Else if input is "terminate":
       21. Send "terminate" message to server
       22. Close connection to server
       23. Terminate client

*/