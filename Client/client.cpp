#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <vector>
#include <limits>

#pragma comment(lib, "ws2_32")

using namespace std;

int main()
{
	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(17777);

	connect(ServerSocket, (sockaddr*)&ServerSockAddr, sizeof(ServerSockAddr));

	srand((unsigned int)time(nullptr));
	int ClientCount = rand() & 10000;

	char Buffer[1024] = "Hello";
	int SendBytes = send(ServerSocket, Buffer, (int)strlen(Buffer) + 1, 0);
	cout << Buffer << endl;

	int RecvBytes = recv(ServerSocket, Buffer, sizeof(Buffer), 0);
	cout << Buffer << endl;

	closesocket(ServerSocket);

	return 0;
}