/**
 * @file	Client.cpp
 * @brief	A basic socket server client.
 *
 * Basic socket server client for Windows, using winsock2.
 *
 * @author	GravitySquid
 * @date	2024/08/25
 */

#include <iostream>
#include <string>
#include <format>
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton
#include <objbase.h>  // For CoCreateGuid

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

#define DATA_BUFSIZE 1024

int main(int argc, char* argv[]) {

	std::cout << "Client started ... " << std::endl;

	const char* ip, * port;
	
	// Check optional command line arguments
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

	// Convert port to ushort
	unsigned short  usPort = (unsigned short)std::stoi(port);

	// Make a unique ID for the client, for display only
	GUID myGuid;
	char clientGuid[40];
	HRESULT hr = CoCreateGuid(&myGuid);
	if (SUCCEEDED(hr))
	{
		wchar_t guidwString[40]; // Sufficient size for the GUID string
		// Convert the GUID to a string
		if (StringFromGUID2(myGuid, guidwString, std::wcslen(guidwString)-1) != 0) {
			size_t len = wcslen(guidwString) + 1;
			char* guidString = new char[len];
			memset(guidString, 0, len);
			size_t cLen;
			wcstombs_s(&cLen, guidString, len, guidwString, len - 1);
			strcpy_s(clientGuid, sizeof(clientGuid), guidString);
			std::cout << "Client ID is ... " << clientGuid << std::endl;
		}
	}
	else
		std::cout << "Could not create GUID for Client " << std::endl;

	// Start WinSock API
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
		return 1;
	}

	// Create a socket for client
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Error creating socket." << std::endl;
		WSACleanup();
		return 1;
	}

	// Populate socket address struct
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

	// Main client loop - keep active
	int i = 0;
	bool serverActive = true;
	while (serverActive)
	{
		// delay for testing, will send periodic message to server
		Sleep(3000);

		// Send data to the server
		//std::string msgSend = std::format("{}: Hey! Server! # {}",clientGuid,i++);
		std::string msgSend;
		std::cout << "\033[32m" << "ENTER MESSAGE FOR SERVER: > " << "\033[0m";
		std::getline(std::cin, msgSend);


		const char* message = msgSend.c_str();
		send(clientSocket, message, strlen(message), 0);
		std::cout << "Sent to server: " << message << std::endl;

		// Receive response data from the server
		char buffer[DATA_BUFSIZE];
		int bytesRead = recv(clientSocket, buffer, DATA_BUFSIZE, 0);
		if (bytesRead > 0) {
			std::string sBuff(buffer);
			std::string msgRecv = sBuff.substr(0, bytesRead);
			std::cout << "Received from server: " << msgRecv << std::endl;
		}
		else
		{
			std::cout << "Server has closed connection ... " << std::endl;
			std::cout << "Exit Client " << std::endl;
			serverActive = false;
		}
	}
	// Clean up
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}

