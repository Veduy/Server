#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <iostream>

#include "Common.h"

#pragma comment(lib, "Common.lib")

using namespace std;

string UserName;


unsigned RecvThread(void* Arg)
{
	SOCKET ServerSocket = *(SOCKET*)Arg;

	char Buffer[1024] = { 0 };

	while (true)
	{
		RecvPacket(ServerSocket, Buffer);

		flatbuffers::FlatBufferBuilder Builder;
		auto UserEvent = UserEvents::GetEventData(Buffer);
		switch (UserEvent->data_type())
		{
		case UserEvents::EventType_S2C_Login:
			auto S2C_Login = UserEvent->data_as_S2C_Login();
			cout << S2C_Login->player_id() << endl;
			cout << S2C_Login->position_x() << endl;
			cout << S2C_Login->position_y() << endl;
			break;
		}
	}

	return 0;
}

unsigned SendThread(void* Arg)
{
	SOCKET ServerSocket = *(SOCKET*)Arg;

	flatbuffers::FlatBufferBuilder Builder;
	auto C2S_LoginData = UserEvents::CreateC2S_Login(Builder, Builder.CreateString("yuyu8488"), Builder.CreateString("1q2w3e4r"));

	auto EventData = UserEvents::CreateEventData(Builder, 0, UserEvents::EventType_C2S_Login, C2S_LoginData.Union());

	Builder.Finish(EventData);

	SendPacket(ServerSocket, Builder);

	return 0;
}


int main()
{
	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	char Buffer[1024] = { 0, };

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(30000);

	int Result = connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	HANDLE SocketThread[2];
	SocketThread[0] = (HANDLE)_beginthreadex(0, 0, RecvThread, &ServerSocket, 0, 0);
	SocketThread[1] = (HANDLE)_beginthreadex(0, 0, SendThread, &ServerSocket, 0, 0);

	DWORD HResult = WaitForMultipleObjects(2, SocketThread, true, INFINITE);

	CloseHandle(SocketThread[0]);
	CloseHandle(SocketThread[1]);

	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}