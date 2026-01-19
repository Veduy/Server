#include <iostream>

#include "Common.h"
#include "httplib.h"

#pragma comment(lib, "Common.lib")

int main()
{
	std::cout << "client start.." << std::endl;

	int Case;
	Case = std::cin.get();

	httplib::Client cli("http://127.0.0.1:8080");
	httplib::Result res;

	if (Case == '1')
	{
		if (res = cli.Get("/api/login")) 
		{
			if (res->status == 200)
			{
				std::cout << "Response: " << res->body << std::endl;
			}
			else
			{
				std::cout << "Error: " << res->status << std::endl;
			}

		}
	}
	else if (Case == '2')
	{
		if (res = cli.Get("/api/check/ranking"))
		{
			if (res->status == 200)
			{
				std::cout << "Response: " << res->body << std::endl;
			}
			else
			{
				std::cout << "Error: " << res->status << std::endl;
			}
		}
	}
	else 
	{
		auto err = res.error();
		std::cout << "HTTP Error: " << httplib::to_string(err) << std::endl;
	}

	// 종료 방지용 대기 코드
    std::cout << "\nPress Enter to exit...";
    std::cin.ignore(); // 버퍼 비우기
    std::cin.get();    // 진짜 대기

	return 0;
}