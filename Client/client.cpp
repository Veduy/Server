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

	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = PF_INET;
	serverSockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverSockAddr.sin_port = htons(8488);


	char sendBuffer[] = "hello server";
	int sendBytes = sendto(serverSocket, sendBuffer, (int)(strlen(sendBuffer) + 1), 0, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));
	if (sendBytes <= 0)
	{

	}

	SOCKADDR_IN remoteSockAddr;
	memset(&remoteSockAddr, 0, sizeof(remoteSockAddr));
	int remoteSockAddrLength = sizeof(remoteSockAddr);

	char recvBuffer[1024] = { 0, };
	int recvBytes = recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0, (sockaddr*)&remoteSockAddr, &remoteSockAddrLength);
	if (recvBytes <= 0)
	{

	}
	printf(recvBuffer);
	
	closesocket(serverSocket);

	WSACleanup();

	return 0;
}