#include "IP.h"
#include "Logger.h"
#include <iostream>


IP::IP(bsoncxx::document::view_or_value Doc) {
    
    ip = Doc.view()["ip"].get_string();
    port = Doc.view()["port"].get_string();
    country = Doc.view()["country"].get_string();
    protocol = Doc.view()["protocols"].get_string();
    responseTime = Doc.view()["responseTime"].get_int32();

    std::cout << "created" << std::endl;
}

void IP::getInfo() {
    std::cout << country << " " << protocol << " " << ip << ":" << port << std::endl;
}

std::string IP::getIP() {
    return ip;
}

std::string IP::getPort() {
    return port;
}

std::string IP::getCountry() {
    return country;
}

std::string IP::getProtocol() {
    return protocol;
}

int IP::getResponseTime() {
    return responseTime;
}