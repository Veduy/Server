#include <iostream>

#include "Common.h"
#include "httplib.h"

#pragma comment(lib, "Common.lib")

int main()
{
	std::cout << "client start.." << std::endl;

	std::cin.get();

	httplib::Client cli("http://127.0.0.1:8080");

	if (auto res = cli.Get("/api/login")) {
		if (res->status == 200) {
			std::cout << "Response: " << res->body << std::endl;
		}
		else {
			std::cout << "Error: " << res->status << std::endl;
		}
	}
	else {
		auto err = res.error();
		std::cout << "HTTP Error: " << httplib::to_string(err) << std::endl;
	}


	return 0;
}