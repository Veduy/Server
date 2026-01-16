#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <iostream>

#include "Common.h"
#include "httplib.h"

#include "mysql-connector/include/jdbc/mysql_connection.h"
#include "mysql-connector/include/jdbc/cppconn/driver.h"
#include "mysql-connector/include/jdbc/cppconn/exception.h"
#include "mysql-connector/include/jdbc/cppconn/resultset.h"
#include "mysql-connector/include/jdbc/cppconn/statement.h"
#include "mysql-connector/include/jdbc/cppconn/prepared_statement.h"

#pragma comment(lib, "mysqlcppconn.lib")
#pragma comment(lib, "Common.lib")

// Helper function to create JSON response (minimal implementation)
std::string CreateJsonResponse(const std::string& name, bool result) {
    if (name.empty()) {
        return "{\"result\": false}";
    }
    return "{\"name\": \"" + name + "\", \"result\": " + (result ? "true" : "false") + "}";
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
        std::cout << Error << std::endl;
    }

    httplib::Server svr;
    
    // MySQL connection settings
    const std::string db_url = "tcp://127.0.0.1:3306";
    const std::string db_user = "root";
    const std::string db_pass = "qweasd123";
    const std::string db_schema = "membership";

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

            try {
                sql::Driver* Driver = get_driver_instance();
                sql::Connection* Connection(Driver->connect(db_url, db_user, db_pass));
                Connection->setSchema(db_schema);

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
  
    std::cout << "WebServer starting at http://127.0.0.1:8080/api/login" << std::endl;
    

    svr.listen("0.0.0.0", 8080); //블로킹 상태 진입함.
    //GateServer로부터 Packet받아야할텐데, 쓰레드 추가해야할듯.

    return 0;
}
