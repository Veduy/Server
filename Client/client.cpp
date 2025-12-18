#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <vector>
#include <limits>

#pragma comment(lib, "ws2_32")

int main()
{
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = PF_INET;
	serverSockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverSockAddr.sin_port = htons(8488);

	connect(serverSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));	
	
	std::string input;

	while (true)
	{
		std::getline(std::cin, input);

		int valueSize = input.size();
		int netSize = htonl(valueSize);
		
		/*패킷 버퍼*/
		std::vector<char> packet(sizeof(netSize) + valueSize);
		memcpy(packet.data(), &netSize, sizeof(netSize));
		memcpy(packet.data() + sizeof(netSize), input.data(), valueSize);

		int totalSent = 0;
		int packetSize = packet.size();
		while (totalSent < packetSize)
		{
			int sentBytes = send(serverSocket, packet.data() + totalSent, packetSize - totalSent, 0);

			if (sentBytes <= 0)
			{
				return false;
			}
			
			totalSent += sentBytes;
		}
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}