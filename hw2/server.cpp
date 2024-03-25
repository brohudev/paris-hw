#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 32768
#define IP_ADDRESS "127.0.0.1" //idk why this is here even though it does nothing
#define BUFFER_SIZE 1024 //change in the other file as well if you want a diff size

class SocketServer{
    private:
        int serverSocket;
    
    public:
        SocketServer(const char*, int);
        ~SocketServer();
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
}
SocketServer::~SocketServer(){
      close(serverSocket);
}

int main() {
    SocketServer server(IP_ADDRESS, PORT);
    std::cout << "Server listening on port 32768\n";
  
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