#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32")

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

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = PF_INET;
	serverSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverSockAddr.sin_port = htons(8488);

	connect(serverSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));

	TransferData incomeData;
	int recvBytes = recv(serverSocket, (char*)&incomeData, sizeof(TransferData), 0);
	std::cout << incomeData.dataCount << std::endl;
	std::cout << incomeData.dataSize[0] << std::endl;
	std::cout << incomeData.dataSize[1] << std::endl;

	for (int i = 0; i < incomeData.dataCount; ++i)
	{
		int fileSize = 0;
		char* buffer = new char[incomeData.dataSize[i]];
		int recvBytes = recv(serverSocket, buffer, incomeData.dataSize[i], MSG_WAITALL);

		char filename[64];
		sprintf(filename, "test%d.jpg", i);

		FILE* file = fopen(filename, "wb");
		int fileWrite = fwrite(buffer, sizeof(char), recvBytes, file);
	}
	
	WSACleanup();
}