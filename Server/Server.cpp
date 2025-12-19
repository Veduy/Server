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
	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	hostent* host = gethostbyname("google.com");

	if (host)
	{
		cout << host->h_name << endl;
	}

	for (int i = 0; host->h_aliases[i]; ++i)
	{
		cout << host->h_aliases[i] << endl;
	}
	

	WSACleanup();

	return 0;
}