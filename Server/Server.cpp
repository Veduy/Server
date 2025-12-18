#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <vector>

#pragma comment(lib, "ws2_32")

using namespace std;

int main()
{
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = PF_INET;
	serverSockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverSockAddr.sin_port = htons(8488);

	bind(serverSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));

	listen(serverSocket, 10);

	while (true)
	{
		sockaddr_in clientSockAddr;
		memset(&clientSockAddr, 0, sizeof(sockaddr_in));
		int clientAddrLen = sizeof(clientSockAddr);

		SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientSockAddr, &clientAddrLen);

		/* client connect accepted */
		while (clientSocket)
		{
			int value = 0;
			int recvBytes = recv(clientSocket, (char*)&value, sizeof(value), 0);
			if (recvBytes > 0)
			{
				printf("%d\n", value);
			}
			if (recvBytes <= 0)
			{
				break;
				closesocket(clientSocket);
			}
		}
	}



	

	
	closesocket(serverSocket);

	WSACleanup();

	return 0;
}