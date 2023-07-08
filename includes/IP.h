#pragma once
#include <bsoncxx/document/view_or_value.hpp>
#include <iostream>

class IP {

public:
	IP(bsoncxx::document::view_or_value Doc);

	// returns Country, ip, port
	void getInfo();

	std::string getIP();
	std::string getPort();
	std::string getCountry();
	std::string getProtocol();
	int getResponseTime();

	~IP() { std::cout << "destroyed" << std::endl; };

private:
	
	std::string ip;
	std::string port;
	std::string country;
	std::string protocol;
	int responseTime;

};