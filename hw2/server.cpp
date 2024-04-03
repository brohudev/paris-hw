#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error: Could not create socket\n";
        return 1;
    }

    // Bind socket to port 8081
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8081);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Could not bind to port\n";
        return 1;
    }

    // Listen
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error: Could not listen on socket\n";
        return 1;
    }

    std::cout << "Server listening on port 8081...\n";

    // Accept incoming connections
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSocket == -1) {
        std::cerr << "Error: Could not accept incoming connection\n";
        return 1;
    }

    // Receive message from client
    char buffer[1024] = {0};
    int valread = read(clientSocket, buffer, 1024);
    std::cout << "Received message from client: " << buffer << std::endl;

    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}


/*
1. Initialize socket for server
2. Bind socket to server address
3. Listen for incoming connections
4. Loop indefinitely:
    5. Accept incoming connection
    8. Loop indefinitely:
        9. Receive message from client
        10. If message is "get filename":
            11. Check if file exists in Repository directory
            12. If file exists:
                13. send the file size and name as a message. 
                15. Print message indicating file sent and its size
            16. Else:
                17. Send message to client indicating file is missing
        18. Else if message is "terminate":
            19. Send "Goodbye!" message to client
            20. Terminate child process
21. Close server socket
*/