/**
 * @file	Client.cpp
 * @brief	A basic socket server client.
 *
 * Basic socket server client for Windows and Linux, using platform-specific socket APIs.
 *
 * Execution example:
 * 1. Start the server (Server.exe) in a terminal.
 * 2. Start the client (Client.exe) in another terminal - "client.exe <IP> <Port>"
 * 
 * @author	GravitySquid
 * @date	2024/08/25
 */

#include <iostream>
#include <string>
#include <format>
#include <cstring>
#include <thread>
#include <chrono>
#include <random>
#include <sstream> 
#include <iomanip>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h> // For inet_pton
	#include <objbase.h>  // For CoCreateGuid
	#pragma comment(lib, "ws2_32.lib") // Link with Winsock library
	typedef int socklen_t;
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	typedef int SOCKET;
#endif

#define DATA_BUFSIZE 1024

// Cross-platform UUID/GUID generator
std::string generateClientId()
{
#ifdef _WIN32
	GUID myGuid;
	HRESULT hr = CoCreateGuid(&myGuid);
	if (SUCCEEDED(hr))
	{
		wchar_t guidwString[40];
		if (StringFromGUID2(myGuid, guidwString, 40) != 0) {
			size_t len = wcslen(guidwString) + 1;
			char* guidString = new char[len];
			memset(guidString, 0, len);
			size_t cLen;
			wcstombs_s(&cLen, guidString, len, guidwString, len - 1);
			std::string result(guidString);
			delete[] guidString;
			return result;
		}
	}
	return "GUID-ERROR";
#else
	// Linux: generate a simple UUID-like string
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 15);
	const char hexChars[] = "0123456789abcdef";
	std::string uuid;
	
	// Format: 8-4-4-4-12 hexadecimal digits
	for (int i = 0; i < 8; ++i) uuid += hexChars[dis(gen)];
	uuid += "-";
	for (int i = 0; i < 4; ++i) uuid += hexChars[dis(gen)];
	uuid += "-";
	for (int i = 0; i < 4; ++i) uuid += hexChars[dis(gen)];
	uuid += "-";
	for (int i = 0; i < 4; ++i) uuid += hexChars[dis(gen)];
	uuid += "-";
	for (int i = 0; i < 12; ++i) uuid += hexChars[dis(gen)];
	
	return uuid;
#endif
}

// Cross-platform sleep function
void platformSleep(int milliseconds)
{
#ifdef _WIN32
	Sleep(milliseconds);
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

// Cross-platform socket close
void closeSocket(SOCKET sock)
{
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif
}

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
	unsigned short usPort = (unsigned short)std::stoi(port);

	// Make a unique ID for the client, for display only
	std::string clientGuid = generateClientId();
	std::cout << "Client ID is ... " << clientGuid << std::endl;

	// Start socket initialization (platform-specific)
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
		return 1;
	}
#endif

	// Create a socket for client
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Error creating socket." << std::endl;
#ifdef _WIN32
		WSACleanup();
#endif
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
		closeSocket(clientSocket);
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	// Main client loop - keep active
	bool serverActive = true;
	while (serverActive)
	{
		// delay for testing, will send periodic message to server
		platformSleep(3000);

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
	closeSocket(clientSocket);
#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}

