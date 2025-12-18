#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32")

int main()
{
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = PF_INET;
	serverSockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverSockAddr.sin_port = htons(8488);

	connect(serverSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));	
	
	srand(time(nullptr));

	int value = 0;

	while (true)
	{
		std::cin >> value;
		send(serverSocket, (char*)&value, sizeof(value), 0);
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}