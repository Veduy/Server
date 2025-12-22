#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <process.h>
#include <iostream>
//#include <WinSock2.h>
#include <string>
#include <vector>
#include <limits>

#pragma comment(lib, "ws2_32")

using namespace std;

unsigned ThreadMessage(void* Arg)
{
	SOCKET Socket = (SOCKET)(Arg);

	while (true)
	{
		char Buffer[1024] = { 0 };
		int RecvBytes = recv(Socket, Buffer, sizeof(Buffer), 0);

		if (RecvBytes > 0)
		{
			cout << Buffer << endl;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int main()
{
	srand((unsigned int)time(nullptr));

	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(17777);

	connect(ServerSocket, (sockaddr*)&ServerSockAddr, sizeof(ServerSockAddr));

	HANDLE MessageHandle = (HANDLE)_beginthreadex(nullptr, 0, ThreadMessage, (void*)ServerSocket, NULL, NULL);

	while (true)
	{
		std::string Buffer;
		getline(std::cin, Buffer);

		int BufferSize = Buffer.size();
		int NetBufferSize = htonl(BufferSize);

		std::vector<char>Packet(sizeof(NetBufferSize) + BufferSize);
		memcpy(Packet.data(), &NetBufferSize, sizeof(int));
		memcpy(Packet.data() + sizeof(int), Buffer.data(), BufferSize);

		int TotalSent = 0;
		int PacketSize = Packet.size();
		while (TotalSent < PacketSize)
		{
			int SentBytes = send(ServerSocket, Packet.data() + TotalSent, PacketSize - TotalSent, 0);

			if (SentBytes <= 0)
			{
				return false;
			}

			TotalSent += SentBytes;
		}		
	}

	closesocket(ServerSocket);

	return 0;
}