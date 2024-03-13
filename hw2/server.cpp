#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *ack_msg = "Message received by server.";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Receiving message from client
    read(new_socket, buffer, BUFFER_SIZE);
    std::cout << "Client: " << buffer << std::endl;

    // Sending acknowledgment to client
    send(new_socket, ack_msg, strlen(ack_msg), 0);
    std::cout << "Acknowledgment sent to client." << std::endl;

    close(new_socket);
    close(server_fd);
    return 0;
}

/*
1. Initialize socket for server
2. Bind socket to server address
3. Listen for incoming connections
4. Loop indefinitely:
   5. Accept incoming connection
   6. Fork a child process to handle client
   7. Inside child process:
      8. Loop indefinitely:
         9. Receive message from client
         10. If message is "get filename":
             11. Check if file exists in Repository directory
             12. If file exists:
                 13. Open file and read its contents
                 14. Send file contents to client
                 15. Print message indicating file sent and its size
             16. Else:
                 17. Send message to client indicating file is missing
         18. Else if message is "terminate":
             19. Send "Goodbye!" message to client
             20. Terminate child process
21. Close server socket

*/