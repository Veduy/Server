#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(SOCKADDR_IN));
	serverSockAddr.sin_family = AF_INET;
	serverSockAddr.sin_addr.s_addr = INADDR_ANY;
	serverSockAddr.sin_port = htons(8488);


	bind(listenSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));

	
	listen(listenSocket, 10);

	sockaddr_in clientSockAddr;
	memset(&clientSockAddr, 0, sizeof(sockaddr_in));
	int clientSocklen = sizeof(clientSockAddr);


	SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientSockAddr, &clientSocklen);

	/*Do Something*/

	FILE* fp;
	fp = fopen("tree.jpg", "rb");

	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	send(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);
	
	char buffer[1024];
	size_t byteRead;
	
	byteRead = fread(buffer, sizeof(char), sizeof(buffer), fp);
	send(clientSocket, buffer, byteRead, 0);

	while ((byteRead = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0)
	{
		size_t totalSent = 0;

		while (totalSent < byteRead)
		{
			int sent = send(clientSocket, buffer + totalSent, byteRead - totalSent, 0);	

			totalSent += sent;
		}

	}

	fclose(fp);

	closesocket(clientSocket);

	closesocket(listenSocket);

	WSACleanup();

	return 0;
}