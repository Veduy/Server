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
	
	srand(time(nullptr));

	std::string input;

	while (true)
	{
		int totalSent = 0;

		std::cin >> input;
		int value = 0;
		try 
		{
			value = std::stol(input);
			if (value > (std::numeric_limits<int>::max)())
			{
				continue;
			}
		}
		catch (const std::out_of_range&)
		{
			printf("out_of_range\n");
			continue;
		}
		catch (const std::invalid_argument&)
		{
			printf("invalid argument\n");
			continue;
		}

		int packetSize = sizeof(value);
		
		char packet[sizeof(packetSize) + sizeof(value)];
		memcpy(packet, &packetSize, sizeof(packetSize));
		memcpy(packet + sizeof(packetSize), &value, sizeof(value));

		while (totalSent < packetSize + sizeof(value))
		{
			int sentBytes = send(serverSocket, (char*)(packet + totalSent), sizeof(packet) - totalSent, 0);

			if (sentBytes <= 0)
			{
				// 에러
				return false;
			}
			
			totalSent += sentBytes;
		}
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}