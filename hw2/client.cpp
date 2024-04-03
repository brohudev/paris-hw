#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define SERVER_PORT 32770

using namespace std;

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
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Connection failed\n";
        return 1;
    }

    cout <<"connected to the server at port: "<<SERVER_PORT<< endl;
    std::string message;

    while (true) {
        std::cout << "Enter message to send to server (type 'terminate' to quit the client and server): ";
        std::getline(std::cin, message);

        if (message == "terminate") { //send exit to server and quit
            send(clientSocket, message.c_str(), message.size(), 0);
            break; // Exit loop if user types 'exit'
        }

        send(clientSocket, message.c_str(), message.size(), 0);
        std::cout << "Message sent to server\n";

        // Receive response from server
        char buffer[1024] = {0};
        int valread = read(clientSocket, buffer, 1024);

        if (strcmp(buffer, "fileNotFound") != 0) {
            std::cout << "Received message from server: " << buffer << std::endl;
        }
    }

    return 0;
}