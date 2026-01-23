#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <iostream>

#include "Common.h"
#include "json.hpp"
#include "httplib.h"

#include <thread>

#include "mysql-connector/include/jdbc/mysql_connection.h"
#include "mysql-connector/include/jdbc/cppconn/driver.h"
#include "mysql-connector/include/jdbc/cppconn/exception.h"
#include "mysql-connector/include/jdbc/cppconn/resultset.h"
#include "mysql-connector/include/jdbc/cppconn/statement.h"
#include "mysql-connector/include/jdbc/cppconn/prepared_statement.h"

#pragma comment(lib, "mysqlcppconn.lib")
#pragma comment(lib, "Common.lib")

using json = nlohmann::json;

void ReceiveMatchResult(SOCKET ServerSocket)
{
    char Buffer[4096] = { 0 };
    while (true)
    {
        int RecvBytes = RecvPacket(ServerSocket, Buffer);
        if (RecvBytes > 0)
        {
            auto MatchData = MatchingEvents::GetMatchData(Buffer);
            std::cout << "\n===== Match Found! =====" << std::endl;
            auto users = MatchData->users();
            for (unsigned int i = 0; i < users->size(); ++i)
            {
                auto user = users->Get(i);
                std::cout << "User " << i + 1 << ": ID(" << user->idx() << "), Name(" << user->name()->c_str() << ")" << std::endl;
            }
            std::cout << "========================\n" << std::endl;
        }
        else if (RecvBytes <= 0)
        {
            std::cout << "GatewayServer disconnected" << std::endl;
            break;
        }
    }
}

int main() 
{
    //GatewayServer와 소켓 연결.
    WSAData WsaData;
    WSAStartup(MAKEWORD(2, 2), &WsaData);

    SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN ServerSockAddr;

    memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
    ServerSockAddr.sin_family = AF_INET;
    ServerSockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    ServerSockAddr.sin_port = htons(18488);

    int ConnectResult = connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

    if (ConnectResult == SOCKET_ERROR)
    {
        int Error = WSAGetLastError();
        std::cout << "GatewayServer Connect Error: " << Error << std::endl;
    }
    else
    {
        std::cout << "Connected to GatewayServer." << std::endl;
        std::thread ReceiveThread(ReceiveMatchResult, ServerSocket);

        //이 스레드는 독립적으로 실행할거임 -> 객체가 없어져도 상관x 
        // std::thread객체는 소멸될때, join()되거나, detach()되어 있지 않으면 프로그램은 강제로 종료.
        ReceiveThread.detach();
    }

    httplib::Server svr;
    
    // MySQL connection settings
    const std::string db_url = "tcp://127.0.0.1:3306";
    const std::string db_user = "root";
    const std::string db_pass = "qweasd123";
    const std::string db_schema = "membership";

    sql::Driver* Driver = get_driver_instance();
    sql::Connection* Connection = nullptr; 

    try
    {
        Connection = Driver->connect(db_url, db_user, db_pass);
        Connection->setSchema(db_schema);
    }
    catch (sql::SQLException& e)
    {
        std::cout << "[DB CONNECT FAIL]\n";
        std::cout << "Error: " << e.what() << std::endl;
        std::cout << "ErrorCode: " << e.getErrorCode() << std::endl;
        std::cout << "SQLState: " << e.getSQLState() << std::endl;

        Connection = nullptr;
    }

    svr.Get("/api/login", [&](const httplib::Request& req, httplib::Response& res) 
    {
        /*std::string user_id = req.get_param_value("user_id");
        std::string passwd = req.get_param_value("passwd");*/
        std::string user_id = "admin";
        std::string passwd = "1234";

        if (user_id.empty() || passwd.empty())
        {
            res.set_content(CreateJsonResponse("", false), "application/json");
            return;
        }

        try
        {
            // Using SHA2(?, 256) for password comparison as per Order.txt
            sql::PreparedStatement* PreparedStatement(Connection->prepareStatement(
                "SELECT `id`, `name` FROM user WHERE `user_id` = ? AND passwd = SHA2(?, 256);"
            ));

            PreparedStatement->setString(1, user_id);
            PreparedStatement->setString(2, passwd);

            sql::ResultSet* res_db(PreparedStatement->executeQuery());

            if (res_db->next())
            {
                int idx = res_db->getInt("id");
                std::string name = res_db->getString("name");

                // 접속을 요청한 클라에게 보내는 메시지.
                res.set_content(CreateJsonResponse(name, true), "application/json");

                // 클라에서 웹서버에 로그인 요청후 웹서버에서 DB에서 정보확인한 이후.
                // 이때 GatewayServer에 누가 접속했다고 알려야지
                flatbuffers::FlatBufferBuilder Builder;

                auto ServerLoginData = UserEvents::CreateServerLogin(Builder, (int32_t)idx, Builder.CreateString(name));

                auto EventData = UserEvents::CreateEventData(Builder, 0, UserEvents::EventType_ServerLogin, ServerLoginData.Union());

                Builder.Finish(EventData);

                int PacketSize = Builder.GetSize() + sizeof(int);
                int SentBytes = SendPacket(ServerSocket, Builder);
                if (SentBytes >= PacketSize)
                {
                    std::cout << "Success sending packet" << std::endl;
                }
            }

            else
            {
                res.set_content(CreateJsonResponse("", false), "application/json");
            }
        }
        catch (sql::SQLException& e)
        {
            std::cerr << "MySQL Error: " << e.what() << std::endl;
            res.status = 500;
            res.set_content("{\"error\": \"Database error\"}", "application/json");
        }
        catch (std::exception& e)
        {
            std::cerr << "General Error: " << e.what() << std::endl;
            res.status = 500;
            res.set_content("{\"error\": \"Internal server error\"}", "application/json");
        }
    });

    svr.Get("/api/check/ranking", [&](const httplib::Request& req, httplib::Response& res)
    {
        try
        {
            sql::PreparedStatement* PreparedStatement(Connection->prepareStatement(
                "SELECT RANK() OVER(ORDER BY score DESC) AS ranking, user_name,score FROM ranking ORDER BY ranking LIMIT 10"));

            sql::ResultSet* res_db(PreparedStatement->executeQuery());

            /*
            * {
                  "name": "John",
                  "age": 30,
                  "city": "New York"
               }
            */
            // JSON 문자열
            json Ranking;
         
            while (res_db->next())
            {
                json Row =
                {
                    {"rank", std::to_string(res_db->getInt("ranking"))},
                    {"name", res_db->getString("user_name")},
                    {"score", std::to_string(res_db->getInt("score"))}
                };
                Ranking.push_back(Row);
            }
            
            //auto j3 = json::parse(Ranking);

            res.set_content(Ranking.dump(), "application/json");

            delete PreparedStatement;
        }
        catch (sql::SQLException& e)
        {
            std::cerr << "MySQL Error (Ranking): " << e.what() << std::endl;
            res.status = 500;
            res.set_content("{\"error\": \"Database error during ranking fetch\"}", "application/json");
        }
    });
  
    std::cout << "WebServer starting at http://127.0.0.1:8080/api/login" << std::endl;
    

    svr.listen("0.0.0.0", 8080); //블로킹 상태 진입함.
    //GateServer로부터 Packet받는 쓰레드 추가.

    return 0;
}