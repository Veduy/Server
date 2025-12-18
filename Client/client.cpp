#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <string>

#pragma comment(lib, "ws2_32")

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(SOCKADDR_IN));
	serverSockAddr.sin_family = AF_INET;
	serverSockAddr.sin_addr.s_addr = inet_addr("127.0.01");
	serverSockAddr.sin_port = htons(8488);

	connect(serverSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));

	int fileSize;
	recv(serverSocket, (char*)&fileSize, sizeof(fileSize), 0);
	printf("fileSize : %d", fileSize);

	char* imageBuffer = (char*)malloc(fileSize);
	memset(imageBuffer, 0, fileSize);

	int totalReceived = 0;

	while (totalReceived < fileSize)
	{
		char tempBuffer[1024];

		int received = recv(serverSocket, tempBuffer, min(1024, fileSize - totalReceived), 0);

		memcpy(imageBuffer + totalReceived, tempBuffer, received);

		totalReceived += received;
	}

	FILE* fp = nullptr;
	fopen_s(&fp, "received.jpg", "wb");

	if (fp)
	{
		fwrite(imageBuffer, 1, fileSize, fp);
		fclose(fp);
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}