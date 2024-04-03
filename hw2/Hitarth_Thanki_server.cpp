#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>

//set these parameters wrt your system. 
#define FILE_DIRECTORY "./files/" 
//client must be on a different port than the one specified bellow:
#define SERVER_PORT_NO 32770 //the port the server will bind itself to,a dn the clients will send messages to
#define BUFFER_SIZE 1024 //change in the other file too. 

using namespace std;

int handleClientRequests(int clientSocket) {
    while (true) {
        char buffer[BUFFER_SIZE] = {0};
        int valread = read(clientSocket, buffer, BUFFER_SIZE); //get message from client
        cout << "Received message from client: " << buffer << std::endl;

        if (strcmp(buffer, "terminate") == 0) {
            std::cout << "Client requested to terminate. Closing connection...\n";
            close(clientSocket); // Close connection
            return -1; // signal main to break
        } else if (strcmp(buffer, "exit") == 0) {
            std::cout << "Client requested to exit. Closing connection...\n";
            close(clientSocket); // Close connection
            return 0; // signal main to keep going
        }

        // Parse message to get filename
        string message(buffer);
        string delimiter = " ";
        size_t pos = message.find(delimiter);
        if (pos == string::npos) {
            std::cerr << "Invalid message format\n";
            close(clientSocket);
            return 1; // Return 1 if invalid message format
        }
        string command = message.substr(0, pos);
        string filename = message.substr(pos + delimiter.length());

        // handle case: command is "get"
        if (command == "get") {
            // Check if the file exists
            std::string filepath = FILE_DIRECTORY + filename;
            int file = open(filepath.c_str(), O_RDONLY);
            if (file == -1) {//send fileNotFound error
                std::cout << "A client requested the file " << filename << ", that file is missing!" << std::endl;
                const char *response = "fileNotFound";
                send(clientSocket, response, strlen(response), 0);
                continue; //restart loop
            } 
            else {// Get file size and send it.
                struct stat fileStat;
                if (fstat(file, &fileStat) == -1) { // send fileNotFound because you cant read file size
                    std::cout << "cant get file size of the file: " << filename << std::endl;
                    const char *response = "fileNotFound";
                    send(clientSocket, response, strlen(response), 0);
                    continue; //restart loop
                }

                std::string fileSize = std::to_string(fileStat.st_size);
                message = filename + " (" + fileSize + " bytes)";
               cout <<"  A client requested the file" << filename << ", Sent " << fileSize << " bytes."<<endl;
                send(clientSocket, message.c_str(), message.size(), 0);

                close(file);
            }
        } else {
            cout << "Invalid command\n"; continue;
        }
    }
}

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
    serverAddr.sin_port = htons(SERVER_PORT_NO);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Could not bind to port\n";
        return 1;
    }

    // Listen
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error: Could not listen on socket\n";
        return 1;
    }

    std::cout << "Server listening on port "<<SERVER_PORT_NO<<"...\n";

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket == -1) {
            std::cerr << "Error: Could not accept incoming connection\n";
            return 1;
        }

        if (handleClientRequests(clientSocket) < 0){ //handle the requests
            break; //the client requested to terminate the server as well. 
        }
        //else continue with a new client connection. 
    }

    close(serverSocket);
    return 0;
}