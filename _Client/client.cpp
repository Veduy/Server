#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

#pragma comment(lib, "ws2_32")

using namespace std;

unsigned RecvThread(void* Arg)
{
	SOCKET Sock = (SOCKET)(Arg);

	while (true)
	{
		char Buffer[1024] = { 0 };
		int RecvBytes = recv(Sock, Buffer, sizeof(Buffer), 0);
		cout << Buffer << endl;
	}
}

unsigned SendThread(void* Arg)
{
	SOCKET Sock = (SOCKET)(Arg);

	while (true)
	{
		char Buffer[1024] = { 0 };
		cin.getline(Buffer, sizeof(Buffer));
		int SendBytes = send(Sock, Buffer, (int)strlen(Buffer) + 1, 0);
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
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(17777);

	connect(ServerSocket, (sockaddr*)&ServerSockAddr, sizeof(ServerSockAddr));

	HANDLE Threads[2];

	Threads[0] = (HANDLE)_beginthreadex(nullptr, 0, RecvThread, (void*)ServerSocket, 0
	, NULL);

	Threads[1] = (HANDLE)_beginthreadex(nullptr, 0, SendThread, (void*)ServerSocket, 0
		, NULL);

	// signaled 
	// 두 쓰레드가 끝날때까지 기다려.
	DWORD Result = WaitForMultipleObjects(2, Threads, true, INFINITE);

	CloseHandle(Threads[0]);
	CloseHandle(Threads[1]);

	closesocket(ServerSocket);

	return 0;
}