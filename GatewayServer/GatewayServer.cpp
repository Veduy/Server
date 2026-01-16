
#include <iostream>

#include "Common.h"

#pragma comment(lib, "Common.lib")

int main()
{
	WSAData WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);

	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(18488);

	int Result = bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	fd_set MasterReadSet;
	FD_ZERO(&MasterReadSet);
	FD_SET(ListenSocket, &MasterReadSet);

	printf("GatewayServer started...\n");

	while (true)
	{
		TIMEVAL TimeOut;
		TimeOut.tv_sec = 0;
		TimeOut.tv_usec = 100;

		fd_set WorkingReadSet = MasterReadSet;
		int SelectCount = select(0, &WorkingReadSet, nullptr, nullptr, &TimeOut);
		if (SelectCount == 0)
		{
			// 타임아웃
			continue;
		}

		for (int i = 0; i < (int)WorkingReadSet.fd_count; ++i)
		{
			SOCKET SelectedSocket = WorkingReadSet.fd_array[i];

			if (FD_ISSET(SelectedSocket, &WorkingReadSet))
			{
				if (SelectedSocket == ListenSocket)
				{
					// 연결요청
					SOCKADDR_IN ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockAddrLength = sizeof(ClientSockAddr);
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);
					if (ClientSocket == INVALID_SOCKET)
					{
						int Error = WSAGetLastError();
						continue;
					}
					 
					FD_SET(ClientSocket, &MasterReadSet);
				}
				else
				{
					// 패킷처리
					char Buffer[4096] = { 0 };
					int RecvBytes = RecvPacket(SelectedSocket, Buffer);

					flatbuffers::FlatBufferBuilder Builder;
					auto UserEvent = UserEvents::GetEventData(Buffer);
					switch (UserEvent->data_type())
					{
					case UserEvents::EventType_ServerLogin:
						auto ServerLoginData = UserEvent->data_as_ServerLogin();
						std::cout << ServerLoginData->idx() << std::endl;
						std::cout << ServerLoginData->name()->c_str() << std::endl;

						//flatbuffers::FlatBufferBuilder LoginBuilder;
						//int id = atoi(ServerLoginData->userid()->c_str());
						//auto ClientLoginData = UserEvents::CreateClientLogin(LoginBuilder, id, true, LoginBuilder.CreateString("Login Accepted..."));
						//auto LoginEventData = UserEvents::CreateEventData(LoginBuilder, 0, UserEvents::EventType_ClientLogin, ClientLoginData.Union());
						//LoginBuilder.Finish(LoginEventData);

						//int PacketSize = LoginBuilder.GetSize() + sizeof(int);
						//int SentBytes = SendPacket(SelectedSocket, LoginBuilder);
						//if (PacketSize == SentBytes)
						//{
						//	std::cout << "Success sending packet" << std::endl;
						//}
						//
						//break;
					}
				}
			}
		}
	}

	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}