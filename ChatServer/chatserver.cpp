#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define FD_SETSIZE			100

#include <iostream>
#include <WinSock2.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <vector>

#include "ChatPacket.h"

#pragma comment(lib, "ws2_32")

using namespace std;
using namespace rapidjson;

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
	ListenSockAddr.sin_port = htons(17777);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	cout << "Server Start..." << endl;

	fd_set ReadSocketList;
	FD_ZERO(&ReadSocketList);
	FD_SET(ListenSocket, &ReadSocketList);


	//[1][1][1]  [1][1]   ->   [1][1]
	TIMEVAL Timeout;
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 100;
	while (true)
	{
		fd_set CopyReadSocketList = ReadSocketList;

		int ChangeCount = select(0, &CopyReadSocketList, nullptr, nullptr, &Timeout);
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
					//자료 온거지
					char Buffer[1024] = { 0, };
					int RecvBytes = recv(SelectSocket, Buffer, sizeof(Buffer), 0);
					if (RecvBytes == 0)
					{
						cout << "client disconnect : " << PrintAddress(SelectSocket) << endl;
						FD_CLR(SelectSocket, &ReadSocketList);
						closesocket(SelectSocket);
					}
					else if (RecvBytes < 0)
					{
						cout << "client error disconnect : " << PrintAddress(SelectSocket) << endl;
						FD_CLR(SelectSocket, &ReadSocketList);
						closesocket(SelectSocket);
					}

					// 헤더를 다 받을때까지 기다림
					int NetPacketSize;
					RecvBytes = recv(SelectSocket, (char*)&NetPacketSize, sizeof(int), MSG_WAITALL);
					int PacketSize = ntohl(NetPacketSize);
					if (RecvBytes <= 0)
					{
						// disconnect, error
					}

					// 실제 패킷 사이즈만큼 다 받을때까지 기다림
					std::vector<char> Packet(PacketSize);
					RecvBytes = recv(SelectSocket, Packet.data(), PacketSize, MSG_WAITALL);
					
					//ChatPacket Data;
					//Data.Parse(Packet.data());
					//cout << Data.UserName + ":" << Data.Message << endl;

					for (int j = 0; j < (int)ReadSocketList.fd_count; ++j)
					{
						if (ListenSocket == ReadSocketList.fd_array[i])
						{
							continue;
						}

						int SentBytes = send(ReadSocketList.fd_array[j], (char*)&PacketSize, sizeof(int), 0);

						SentBytes = send(ReadSocketList.fd_array[j], Buffer, RecvBytes, 0);
					}
					
				}
			}
		}
	}

	closesocket(ListenSocket);


	WSACleanup();


	return 0;

}