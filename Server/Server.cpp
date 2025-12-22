#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <string>
#include <map>

#pragma comment(lib, "ws2_32")

using namespace std;

int main()
{
	WSAData WsaData;

	WSAStartup(MAKEWORD(2, 2), &WsaData);

	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSocket, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = PF_INET;
	ListenSockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(17777);

	bind(ListenSocket, (sockaddr*)&ListenSockAddr, sizeof(ListenSockAddr));
	listen(ListenSocket, 10);

	FD_SET ReadSocketList;
	FD_ZERO(&ReadSocketList);

	FD_SET(ListenSocket, &ReadSocketList);

	TIMEVAL TiemOut{ 0, 100 };

	while (true)
	{
		FD_SET CopyReadSocketList = ReadSocketList;
		int ChangeCount = select(0, &CopyReadSocketList, nullptr, nullptr, &TiemOut);

		if (ChangeCount == 0)
		{
			cout << "Wait" << endl;
			continue;
		}

		for (int i = 0; i < (int)ReadSocketList.fd_count; ++i)
		{
			SOCKET SelectSocket = ReadSocketList.fd_array[i];

			if (FD_ISSET(SelectSocket, &CopyReadSocketList))
			{
				if (SelectSocket == ListenSocket)
				{
					// 연결요청
					SOCKADDR_IN ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockAddrLength = sizeof(ClientSockAddr);

					SOCKET ClientSocket = accept(ListenSocket, (sockaddr*)&ClientSockAddr, &ClientSockAddrLength);

					FD_SET(ClientSocket, &ReadSocketList);

					cout << "Client connected " << inet_ntoa(ClientSockAddr.sin_addr) << endl;
				}
				else
				{
					// 데이터 수신
					char Buffer[1024] = { 0 };
					int RecvBytes = recv(SelectSocket, Buffer, sizeof(Buffer), 0);
					if (RecvBytes == 0)
					{
						cout << "Client disconnect : " << SelectSocket << endl;
						FD_CLR(SelectSocket, &ReadSocketList);
						closesocket(SelectSocket);
					}
					else if(RecvBytes < 0)
					{
						cout << "Client error disconnect : " << SelectSocket << endl;
						FD_CLR(SelectSocket, &ReadSocketList);
						closesocket(SelectSocket);
					}

					cout << "Client send : " << Buffer << endl;

					int SendBytes = send(SelectSocket, Buffer, RecvBytes, 0);
				}
			}
		}
	}

	closesocket(ListenSocket);

	WSACleanup();

	return 0;
}