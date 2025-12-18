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

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = PF_INET;
	serverSockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverSockAddr.sin_port = htons(8488);

	bind(listenSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));

	listen(listenSocket, 5);
	
	sockaddr_in clientSockAddr;
	memset(&clientSockAddr, 0, sizeof(clientSockAddr));
	int clientSockAddrLen = sizeof(clientSockAddr);

	SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientSockAddr, &clientSockAddrLen);

	char Buffer[1024] = { 0 };
	
	// 파일 읽기
	std::vector<FILE*> files;

	FILE* tree = fopen("tree.jpg", "rb");
	fseek(tree, 0, SEEK_END);
	int fileSize = ftell(tree);
	fseek(tree, 0, SEEK_SET);
	files.push_back(tree);
	
	FILE* gorush = fopen("Gorush.png", "rb");
	fseek(gorush, 0, SEEK_END);
	int fileSize2 = ftell(gorush);
	fseek(gorush, 0, SEEK_SET);
	files.push_back(gorush);

	// 크기 >> 받는다 크기만큼.
	TransferData DataInfo;
	DataInfo.dataCount = files.size();
	DataInfo.dataSize[0] = fileSize;
	DataInfo.dataSize[1] = fileSize2;

	int sendByte = send(clientSocket, (char*)&DataInfo, sizeof(TransferData), 0);

	for (const auto& file : files)
	{
		size_t readByte;
		char buffer[1024];
		do
		{
			int readByte = fread(buffer, sizeof(char), 4, file);
			int sentByte = send(clientSocket, Buffer, readByte, 0);
			if (sentByte < 0)
			{
				closesocket(clientSocket);
			}
		} while (!feof(file));
	}

	fclose(tree);
	fclose(gorush);
	
	//int dataByteSize = htonl(fileSize);
	//int sendByte = send(clientSocket, (char*)&fileSize, sizeof(int), 0);


	WSACleanup();

	return 0;
}