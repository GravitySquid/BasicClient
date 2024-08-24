#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton


#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

int main(int argc, char* argv[]) {

    std::cout << "This is the Client." << std::endl;

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
    // Set to localhost (IPv4 loopback address)
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) != 1) {
        std::cerr << "Error setting address to localhost." << std::endl;
        return 1;
    }
    serverAddress.sin_port = htons(27016);


    // Convert the binary IP address to a string
    char ipAddressStr[INET_ADDRSTRLEN]; // Buffer for IP address string
    if (inet_ntop(AF_INET, &(serverAddress.sin_addr), ipAddressStr, INET_ADDRSTRLEN)) {
        std::cout << "IP Address: " << ipAddressStr << std::endl;
    }
    else {
        std::cerr << "Error converting IP address to string." << std::endl;
    }
    // Convert the port number to a string
    std::cout << "Port: " << ntohs(serverAddress.sin_port) << std::endl;

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
