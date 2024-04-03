#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error: Could not create socket\n";
        return 1;
    }

    // Connect to server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Connection failed\n";
        return 1;
    }

    // Send message to server
    std::cout << "Enter message to send to server: ";
    std::string message;
    std::getline(std::cin, message);

    send(clientSocket, message.c_str(), message.size(), 0);
    std::cout << "Message sent to server\n";

    // Close socket
    close(clientSocket);

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