#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#include "Common.h"

#pragma comment(lib, "Common.lib")


int main()
{
	std::cout << "client start.." << std::endl;

	WSAData WsaData;

	WSAStartup(MAKEWORD(2, 2), &WsaData);

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN ServerSockAddr;
	
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(18488);

	int ConnectResult = connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	if (ConnectResult == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		std::cout << error << std::endl;
	}
	else
	{
		// 로그인 패킷 보낼거임
		flatbuffers::FlatBufferBuilder Builder;
		
		auto ServerLoginData = UserEvents::CreateServerLogin(Builder, Builder.CreateString("admin"), Builder.CreateString("1234"));
		
		auto EventData = UserEvents::CreateEventData(Builder, 0, UserEvents::EventType_ServerLogin, ServerLoginData.Union());

		Builder.Finish(EventData);

		int PacketSize = Builder.GetSize() + sizeof(int);
		int SentBytes = SendPacket(ServerSocket, Builder);
		if (PacketSize == SentBytes)
		{
			std::cout << "Success sending packet" << std::endl;
		}
	}

	return 0;
}