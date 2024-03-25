#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sys/wait.h>
using namespace std;

#define PORT 32768
#define IP_ADDRESS "127.0.0.1" //idk why this is here even though it does nothing
#define BUFFER_SIZE 1024 //change in the other file as well if you want a diff size
#define REPOSITORY_DIRECTORY "~/Repository/"

class SocketServer{
    private:
        int serverSocket;
    
    public:
        SocketServer(const char*, int);
        ~SocketServer();
        void handleClient(int clientSocket);
        void listenForConnection();
};

SocketServer::SocketServer(const char* ip_address, int port_no){
     // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return;
    }

    // Bind to port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_no); // Port number
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding to port\n";
        close(serverSocket);
        return;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening for connections\n";
        close(serverSocket);
        return;
    }

}
SocketServer::~SocketServer(){
      close(serverSocket);
}
void SocketServer::handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    // Receive message from client
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesReceived] = '\0';
        std::string message(buffer);

        if (message.find("get ") == 0) {
            // Extract filename
            std::string filename = message.substr(4);
            std::string filepath = std::string(REPOSITORY_DIRECTORY) + filename;

            // Check if file exists
            std::ifstream file(filepath, std::ios::binary);
            if (file) {
                // File exists
                std::cout << "File found: " << filename << std::endl;

                // Send file contents to client
                while (file.read(buffer, sizeof(buffer))) {
                    send(clientSocket, buffer, file.gcount(), 0);
                }

                // Print message indicating file sent and its size
                std::cout << "File sent: " << filename << " (" << file.tellg() << " bytes)\n";

                // Close file
                file.close();
            } else {
                // File doesn't exist
                std::cout << "File not found: " << filename << std::endl;

                // Send message to client indicating file is missing
                send(clientSocket, "FileNotFound", strlen("FileNotFound"), 0);
            }
        } else if (message == "terminate") {
            // Send "Goodbye!" message to client
            send(clientSocket, "Goodbye!", strlen("Goodbye!"), 0);
            
            // Terminate child process
            close(clientSocket);
            exit(0);
        }
    }

    if (bytesReceived == 0) {
        std::cout << "Client disconnected\n";
    } else {
        std::cerr << "Error receiving message from client\n";
    }

    close(clientSocket);
    exit(1);
}
void SocketServer::listenForConnection(){
    // Accept incoming connection
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        std::cerr << "Error accepting connection\n";
        return;
    }

    // Fork a child process to handle client
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Error forking process\n";
        close(clientSocket);
        return;
    } else if (pid == 0) {
        // Inside child process
        close(serverSocket);
        handleClient(clientSocket);
        close(clientSocket);
        ;
    } else {
        // Inside parent process
        close(clientSocket);
        // Wait for child process to terminate
        while (waitpid(-1, NULL, 1) > 0);
    }
};

int main() {
    SocketServer server(IP_ADDRESS, PORT);


    std::cout << "Server listening on port " << PORT << std::endl;

    // Loop indefinitely
    while (true) {
        server.listenForConnection();
    }

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