#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <atomic>

#pragma comment(lib, "ws2_32")

class SpinLock
{
public:
	void Lock()
	{
		bool expected = false;
		while (!locked.compare_exchange_weak(expected, true, std::memory_order_acquire))
		{
			expected = false;
		}		
	}

	void UnLock()
	{
		locked.store(false, std::memory_order_release);
	}

protected:
	std::atomic_bool(locked);

};



using namespace std;
string PrintAddress(SOCKET InSocket);

CRITICAL_SECTION WorkerSection;
map<SOCKET, SOCKADDR_IN> PlayerList;

unsigned WorkerThread(void* Arg)
{
	SOCKET ClientSocket = *(SOCKET*)Arg;
	
	// 데이터 수신
	while (true)
	{
		char Buffer[1024] = { 0 };
		int RecvBytes = recv(ClientSocket, Buffer, sizeof(Buffer), 0);
		if (RecvBytes == 0)
		{
			cout << "Client disconnect : " << PrintAddress(ClientSocket) << endl;
			closesocket(ClientSocket);

			EnterCriticalSection(&WorkerSection);
			PlayerList.erase(ClientSocket);
			LeaveCriticalSection(&WorkerSection);

			break;
		}
		else if (RecvBytes < 0)
		{
			cout << "Client error disconnect : " << PrintAddress(ClientSocket) << endl;
			closesocket(ClientSocket);

			EnterCriticalSection(&WorkerSection);
			PlayerList.erase(ClientSocket);
			LeaveCriticalSection(&WorkerSection);

			break;
		}

		EnterCriticalSection(&WorkerSection);
		for (const auto& Pair : PlayerList)
		{
			int SendBytes = send(Pair.first, Buffer, RecvBytes, 0);
		}
		LeaveCriticalSection(&WorkerSection);
	}

	return 0;
}

string PrintAddress(SOCKET InSocket)
{
	SOCKADDR_IN OutSocketAddr;
	memset(&OutSocketAddr, 0, sizeof(OutSocketAddr));
	int OutSocketAddrLen = sizeof(OutSocketAddr);

	getpeername(InSocket, (SOCKADDR*)&OutSocketAddr, &OutSocketAddrLen);

	char Buffer[1024] = { 0 };
	sprintf(Buffer, "%s : %d", inet_ntoa(OutSocketAddr.sin_addr), ntohs(OutSocketAddr.sin_port));

	return Buffer;
}

int main()
{

	InitializeCriticalSection(&WorkerSection);

	WSAData WsaData;

	WSAStartup(MAKEWORD(2, 2), &WsaData);

	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = PF_INET;
	ListenSockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(17777);

	bind(ListenSocket, (sockaddr*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 10);

	cout << "Server Start" << endl;

	FD_SET ReadSocketList;
	FD_ZERO(&ReadSocketList);

	FD_SET(ListenSocket, &ReadSocketList);

	SOCKADDR_IN ClientSockAddr;
	memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
	int ClientSockAddrLength = sizeof(ClientSockAddr);

	while (true)
	{
		SOCKET ClientSocket = accept(ListenSocket, (sockaddr*)&ClientSockAddr, &ClientSockAddrLength);
		cout << "Client connected : " << PrintAddress(ClientSocket) << endl;

		EnterCriticalSection(&WorkerSection);
		PlayerList[ClientSocket] = ClientSockAddr;
		LeaveCriticalSection(&WorkerSection);

		// 워커쓰레드로 계속 클라이언트를 추가
		_beginthreadex(nullptr, 0, WorkerThread, &ClientSocket, 0, 0);

		FD_SET CopyReadSocketList = ReadSocketList;

		for (int i = 0; i < (int)ReadSocketList.fd_count; ++i)
		{
			SOCKET SelectSocket = ReadSocketList.fd_array[i];
		}
	}

	closesocket(ListenSocket);

	WSACleanup();

	DeleteCriticalSection(&WorkerSection);

	return 0;
}