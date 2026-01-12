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

		std::string InputUserID;
		std::cout << "UserID: ";
		std::cin >> InputUserID;

		std::cout << "Password: ";
		std::string InputPassword;
		std::cin >> InputPassword;

		auto ServerLoginData = UserEvents::CreateServerLogin(Builder, Builder.CreateString(InputUserID), Builder.CreateString(InputPassword));

		auto EventData = UserEvents::CreateEventData(Builder, 0, UserEvents::EventType_ServerLogin, ServerLoginData.Union());

		Builder.Finish(EventData);

		int PacketSize = Builder.GetSize() + sizeof(int);
		int SentBytes = SendPacket(ServerSocket, Builder);
		if (SentBytes >= PacketSize)
		{
			std::cout << "Success sending packet" << std::endl;
		}
	}

	while (1)
	{
		char Buffer[4096] = { 0 };
		int RecvBytes = RecvPacket(ServerSocket, Buffer);

		flatbuffers::FlatBufferBuilder Builder;
		auto UserEvent = UserEvents::GetEventData(Buffer);
		switch (UserEvent->data_type())
		{
		case UserEvents::EventType_ClientLogin:
			auto ClientLoginData = UserEvent->data_as_ClientLogin();
			std::cout << ClientLoginData->player_id() << std::endl;
			std::cout << ClientLoginData->success() << std::endl;
			std::cout << ClientLoginData->message()->c_str() << std::endl;
			break;
		}
	}
	return 0;
}