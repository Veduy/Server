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

	SOCKET ServerSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(8488);

	char Buffer[1024] = "size";
	int SendBytes = sendto(ServerSocket, Buffer, (int)(strlen(Buffer) + 1), 0, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	cout << Buffer << endl;

	return 0;
}