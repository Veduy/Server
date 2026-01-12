
#include <iostream>

#include "Common.h"

#include "mysql-connector/include/jdbc/mysql_connection.h"
#include "mysql-connector/include/jdbc/cppconn/driver.h"
#include "mysql-connector/include/jdbc/cppconn/exception.h"
#include "mysql-connector/include/jdbc/cppconn/resultset.h"
#include "mysql-connector/include/jdbc/cppconn/statement.h"
#include "mysql-connector/include/jdbc/cppconn/prepared_statement.h"

#pragma comment(lib, "Common.lib")

#pragma comment(lib, "mysqlcppconn.lib")

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

	printf("server started...\n");

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
						std::cout << ServerLoginData->userid()->c_str() << std::endl;
						std::cout << ServerLoginData->password()->c_str() << std::endl;
						// 여기서 DB랑 통신
						
						
						sql::Driver* Driver = nullptr;
						sql::Connection* Connection = nullptr;
						sql::Statement* Statement = nullptr;
						sql::ResultSet* ResultSet = nullptr;
						sql::PreparedStatement* PreparedStatement = nullptr;

						try
						{
							Driver = get_driver_instance();
							Connection = Driver->connect("tcp://127.0.0.1", "root", "qweasd123");

							Connection->setSchema("membership");

							// 동적 쿼리
							PreparedStatement = Connection->prepareStatement("select `passwd` from user where `user_id` = ?;");
							PreparedStatement->setString(1, ServerLoginData->userid()->c_str());
							ResultSet = PreparedStatement->executeQuery();

							while (ResultSet->next())
							{
								std::cout << ResultSet->getString("passwd") << std::endl;
							}

						}
						catch (std::exception e)
						{
							std::cout << e.what() << std::endl;
						}

						delete Statement;
						delete Connection;
					}
				}
			}
		}
	}

	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}