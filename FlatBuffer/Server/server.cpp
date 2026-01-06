#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define FD_SETSIZE			100

#include <iostream>
#include "Common.h"

#pragma comment(lib, "Common.lib")

using namespace std;

void ProcessPacket(SOCKET ClientSocket, const char* Buffer);

string PrintAddress(SOCKET InSocket)
{
	SOCKADDR_IN GetSocketAddr;
	memset(&GetSocketAddr, 0, sizeof(GetSocketAddr));
	int GetSocketAddrLength = sizeof(GetSocketAddr);

	getpeername(InSocket, (SOCKADDR*)&GetSocketAddr, &GetSocketAddrLength);

	char Buffer[1024];
	sprintf(Buffer, "%s:%d", inet_ntoa(GetSocketAddr.sin_addr), ntohs(GetSocketAddr.sin_port));

	return Buffer;
}

int main()
{
	WSAData WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(30000);

	int Result = ::bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	::listen(ListenSocket, 5);

	fd_set ReadSocketList;
	FD_ZERO(&ReadSocketList);
	FD_SET(ListenSocket, &ReadSocketList);

	cout << "Start Server" << endl;

	//[1][1][1]  [1][1]   ->   [1][1]
	TIMEVAL Timeout;
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 100;
	while (true)
	{
		fd_set CopyReadSocketList = ReadSocketList;

		int ChangeCount = ::select(0, &CopyReadSocketList, nullptr, nullptr, &Timeout);
		if (ChangeCount == 0)
		{
			//cout << "Wait" << endl;
			//입력 없을때 서버 작업
			continue;
		}

		//실제 연결처리 및 패킷 처리
		for (int i = 0; i < (int)ReadSocketList.fd_count; ++i)
		{
			SOCKET SelectSocket = ReadSocketList.fd_array[i];
			if (FD_ISSET(SelectSocket, &CopyReadSocketList))
			{
				if (SelectSocket == ListenSocket)
				{
					//연결 요청
					SOCKADDR_IN ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockAddrLength = sizeof(ClientSockAddr);
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);
					FD_SET(ClientSocket, &ReadSocketList);
					cout << "Client connect : " << PrintAddress(ClientSocket) << endl;
				}
				else
				{
					char Buffer[4096] = { 0 };
					int RecvBytes = RecvPacket(SelectSocket, Buffer);

					// Deserialize
					ProcessPacket(SelectSocket, Buffer);
				}
			}
		}
	}

	closesocket(ListenSocket);

	WSACleanup();

	return 0;
}

void ProcessPacket(SOCKET ClientSocket, const char* Buffer)
{
	flatbuffers::FlatBufferBuilder Builder;

	auto RecvEventsData = UserEvents::GetEventData(Buffer);
	
	switch (RecvEventsData->data_type())
	{
	case UserEvents::EventType_C2S_Login:
		auto C2SLoginData = RecvEventsData->data_as_C2S_Login();
		cout << "userid: " << C2SLoginData->userid()->c_str() << endl;
		cout << "password: " << C2SLoginData->password()->c_str() << endl;

		UserEvents::Color MyColor(128, 128, 128);

		auto S2C_LoginData = UserEvents::CreateS2C_Login(Builder, 100, true, Builder.CreateString("success"), 123, 456, &MyColor);

		auto EventData = UserEvents::CreateEventData(Builder, 0, UserEvents::EventType_S2C_Login, S2C_LoginData.Union());

		Builder.Finish(EventData);

		SendPacket(ClientSocket, Builder);

		break;
	}
}
