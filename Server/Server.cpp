#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <map>

#pragma comment(lib, "ws2_32")

using namespace std;

struct ClientSession
{
	SOCKET sock;
	std::vector<char> recvBuffer;
};

int main()
{
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	sockaddr_in serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = PF_INET;
	serverSockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverSockAddr.sin_port = htons(8488);

	bind(serverSocket, (sockaddr*)&serverSockAddr, sizeof(serverSockAddr));
	listen(serverSocket, 10);

	printf("server opend\n");
	
	// client sessions
	std::map<SOCKET, ClientSession> sessions;

	// 살아있는 모든 소켓 목록(서버(1) + 클라이언트(1~63))
	fd_set masterSet;
	FD_ZERO(&masterSet);

	/*서버 리슨소켓 등록*/
	FD_SET(serverSocket, &masterSet);

	while (true)
	{		
		fd_set readSet = masterSet;
		fd_set writeSet;
		fd_set excepSet;

		// 1초마다 OS에 물어봄.
		// 읽을수 있는 소켓이 있음?
		timeval tv{ 1,0 };
		// 소켓들중에서 데이터가 도착했거나, 연결요청이 들어온게 있으면 알려줘.
		int ret = select(0, &readSet, nullptr, nullptr, &tv);
		if (ret <= 0)
		{
			// ret == 0 타임아웃,
			// ret > 0 readSet에 준비된 소켓만 남김.
			continue;
		}

		/*모든 소켓 검사*/
		for (auto i = 0; i < readSet.fd_count; ++i)
		{
			SOCKET sock = readSet.fd_array[i];

			// 서버소켓이라면 새 연결요청
			if (sock == serverSocket)
			{
				sockaddr_in clientAddr;
				memset(&clientAddr, 0, sizeof(sockaddr_in));
				int clientAddrLen = sizeof(clientAddr);

				// select에서 이미 연결을 확인한 상태라 여기 accept는 블로킹 없음 
				SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
				if (clientSocket != INVALID_SOCKET)
				{
					// 신규 client 정보 생성
					FD_SET(clientSocket, &masterSet);

					ClientSession session;
					session.sock = clientSocket;
					sessions.emplace(clientSocket, std::move(session));

					printf("new clientSocket : %d\n", (int)clientSocket);
					printf("new client connected, current connected clients: %d\n", masterSet.fd_count - 1);
				}
			}
			/*클라이언트 소켓이면 데이터수신*/
			else
			{
				auto it = sessions.find(sock);
				if (it == sessions.end())
				{
					continue;
				}
				// 가독성과, 접근비용 때문에 임시 변수로서 저장함.
				ClientSession& session = it->second;

				// 커널->유저 메모리로 복사
				char temp[1024];
				int recvBytes = recv(sock, temp, sizeof(temp), 0);

				if (recvBytes <= 0)
				{
					// recv() == 0	상대가 정상적으로 연결을 종료함.
					// recv() == -1 아직 데이터가 안옴.
					// recv() <	0	에러(랜뽑)
					closesocket(sock);
					FD_CLR(sock, &masterSet);
					sessions.erase(sock);
					printf("client disconnected, current connected clients: %d\n", masterSet.fd_count - 1);
					continue;
				}

				/*데이터 누적해서 받기*/ 
				// recvBytes가 5라면, [0][1][2][3][4][?][?][?]...[?] 이렇게 데이터가 있을거고,
				// 복사할 데이터의 first 0x1234
				// 복사할 데이터의 last   0x1239
				// last 포함 안됨, 범위표현은 항상 반열린 구간 [start, end))
				session.recvBuffer.insert(session.recvBuffer.end(), temp, temp + recvBytes);

				while (true)
				{
					// 약속한 처음 [][][][] 4바이트가 packetSize인데, 그만큼 데이터가 안왔으면 break;
					if (session.recvBuffer.size() < sizeof(int))
					{
						break;
					}

					// memmove는 데이터가 겹칠때, memcpy는 안 겹칠때
					// memmove 뒤->앞, 앞->뒤, memcpy 무조건 앞에서부터
					int packetSize = 0;
					memmove(&packetSize, session.recvBuffer.data(), sizeof(int));
					memcpy(&packetSize, session.recvBuffer.data(), sizeof(int));

					if (session.recvBuffer.size() < sizeof(int) + packetSize)
					{
						break;
					}

					int value = 0;
					memcpy(&value, session.recvBuffer.data() + sizeof(int), packetSize);
					value = ntohl(value);

					printf("received value: %d\n", value);

					session.recvBuffer.erase(session.recvBuffer.begin(), session.recvBuffer.begin() + sizeof(int) + packetSize
					);
				}
			}
		}
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}