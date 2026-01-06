#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "ChatPacket.h"

#pragma comment(lib, "ws2_32")

using namespace std;
using namespace rapidjson;

std::string UserName;

unsigned RecvThread(void* Arg)
{
	SOCKET Sock = (SOCKET)(Arg);

	while (true)
	{
		// 헤더를 다 받을때까지 기다림
		int NetPacketSize;
		int RecvBytes = recv(Sock, (char*)&NetPacketSize, sizeof(int), MSG_WAITALL);
		int PacketSize = ntohl(NetPacketSize);
		if (RecvBytes <= 0)
		{
			// disconnect, error
		}

		// 실제 패킷 사이즈만큼 다 받을때까지 기다림
		std::vector<char> Packet(PacketSize);
		RecvBytes = recv(Sock, Packet.data(), PacketSize, MSG_WAITALL);

		ChatPacket Data;
		Data.Parse(Packet.data());
		cout << Data.UserName + ":" << Data.Message << endl;
	}
}

unsigned SendThread(void* Arg)
{
	SOCKET Sock = (SOCKET)(Arg);

	while (true)
	{
		char Buffer[1024] = { 0 };
		cin.getline(Buffer, sizeof(Buffer));

		ChatPacket Message;
		Message.UserName = "won";
		Message.Message = Buffer;

		// Json 패킷 크기
		std::string Json = Message.ToJsonString();
		int PacketSize = (int)Json.length();
		int NetPacketSize = htonl(PacketSize);
		
		send(Sock, (char*)&NetPacketSize, sizeof(int), 0);
		// SendBytes가 PacketSize보다 작으면 잘라서 계속 보냄.

		send(Sock, Json.data(), PacketSize + 1, 0);
		// SendBytes가 PacketSize보다 작으면 잘라서 계속 보냄.
	}
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
	//ServerSockAddr.sin_addr.s_addr = inet_addr("218.156.17.164");
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(17777);

	connect(ServerSocket, (sockaddr*)&ServerSockAddr, sizeof(ServerSockAddr));

	HANDLE Threads[2];

	Threads[0] = (HANDLE)_beginthreadex(nullptr, 0, RecvThread, (void*)ServerSocket, 0, NULL);

	Threads[1] = (HANDLE)_beginthreadex(nullptr, 0, SendThread, (void*)ServerSocket, 0, NULL);

	// signaled 
	// 두 쓰레드가 끝날때까지 기다려.
	DWORD Result = WaitForMultipleObjects(2, Threads, true, INFINITE);

	CloseHandle(Threads[0]);
	CloseHandle(Threads[1]);

	closesocket(ServerSocket);

	return 0;
}