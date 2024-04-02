#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
using namespace std;

#define PORT_NO 32768
#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1024 //change in the other file as well.

class SocketClient {
    private:
        int socket_fd;

    public:
        SocketClient(const char*, int);
        ~SocketClient();
        void sendRequest(const string&);
        void receiveFile(const string&);
};

SocketClient::SocketClient(const char* ip_address, int port_no){
        // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        std::cerr << "Error creating socket\n";
        return;
    }

    // Connect to server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_no);
    serverAddr.sin_addr.s_addr = inet_addr(ip_address);

    if (connect(socket_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server\n";
        close(socket_fd);
        return;
    }

    std::cout << "Connected to server\n";
}
SocketClient::~SocketClient(){
    // Close socket
    close(socket_fd);
}
void SocketClient::sendRequest(const std::string& request) {
    int makeRequest = send(socket_fd, request.c_str(), request.size(), 0);
    
    if (makeRequest == -1) {
        std::cerr << "Error sending request to server\n";
        return;
    }
}
void SocketClient::receiveFile(const std::string& filename) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    
    std::ofstream outfile(filename, std::ios::binary); //open file
    if (!outfile) {
        std::cerr << "Error opening file for writing\n";
        return;
    }

    while ((bytesReceived = recv(socket_fd, buffer, sizeof(buffer), 0)) > 0) { //write received bytes to file
        outfile.write(buffer, bytesReceived);
    }

    if (bytesReceived == -1) {
        outfile.close();
        if (remove(filename.c_str()) != 0) { //clean up the file of any erroneous bits
            std::cerr << "Error cleaning up file\n";
        }

        std::cerr << "Error receiving file from server\n";
        return;
    }

    // Close file
    outfile.close();
    std::cout << "File received: " << filename << " (" << bytesReceived << " bytes)\n";
}
int main() {
    SocketClient client(IP_ADDRESS, PORT_NO); //wrap everything to clean up main 
    cout << "Client started on port "<< PORT_NO << endl;

    while (true) {
        string input;
        cout << "Enter your request: ";
        getline(std::cin, input);
        //make the code more readable :D
        string command = input.substr(0, input.find_first_of(" "));

        if (command == "get") {
            string fileName = input.substr(input.find_first_of(" ")+1);
            cout << "attempting to get file: " << fileName <<endl;

            client.sendRequest(fileName); //only send in the name bit of the input

            client.receiveFile(fileName);
        } else if (input == "exit") {
            client.sendRequest("terminate");
            return 0; //send message and close
        } else if (input == "terminate") {
            return 0; //close dont send message
        } else {
            std::cout << "Invalid request. Please try again.\n";
        }
    }
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