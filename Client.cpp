#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton


#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

int main(int argc, char* argv[]) {

    std::cout << "Client started ... " << std::endl;
    
    const char *ip, *port;
    if (argc == 3) // Expect IP & port number
    {
        ip = argv[1];
        port = argv[2];
    }
    else // defaults
    {
        std::cout << "Default parameters used ... " << std::endl;
        ip = "127.0.0.1"; // localhost (IPv4 loopback address)
        port = "27016";
    }
    std::cout << "IP to connect to ..... " << ip << std::endl;
    std::cout << "Port to connect to ... " << port << std::endl;

    // Convert port
    unsigned short  usPort = (unsigned short) std::stoi(port);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &(serverAddress.sin_addr)) != 1) {
        std::cerr << "Error setting address to localhost." << std::endl;
        return 1;
    }
    serverAddress.sin_port = htons(usPort);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Send data to the server
    const char* message = "Hello, server!";
    send(clientSocket, message, strlen(message), 0);
    std::cout << "Sent to server: " << message << std::endl;

    // Receive data from the server
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the received data
        std::cout << "Received from server: " << buffer << std::endl;
    }

    // Clean up
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
