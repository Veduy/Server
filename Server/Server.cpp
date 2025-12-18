#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <vector>

#pragma comment(lib, "ws2_32")

using namespace std;

#pragma pack(push, 1)
struct TransferData
{
	size_t dataCount = 0;
	int dataSize[1024] = { 0 };
};
#pragma pack(pop)

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

	bind(serverSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));

	sockaddr_in clientSockAddr;
	memset(&clientSockAddr, 0, sizeof(clientSockAddr));
	int clientSockAddrLength = sizeof(clientSockAddr);

	char buffer[1024] = { 0, };
	int recvBytes = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientSockAddr, &clientSockAddrLength);
	if (recvBytes <= 0)
	{

	}

	printf(buffer);

	char sendBuffer[] = "hello client";
	int sendBytes = sendto(serverSocket, sendBuffer, sizeof(sendBuffer), 0, (sockaddr*)&clientSockAddr, sizeof(clientSockAddr));
	if (sendBytes <= 0)
	{
		
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}