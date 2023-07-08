#pragma once
#include "IP.h"
#include "Logger.h"
#include <curl/curl.h>
#include <string>
#include "MongoDB.h"

class CURLing {
    
private:
    Logger log;
    CURL* curl;
    CURLcode res;
    
    std::string readBuffer;
    std::string ip = "";
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    /// curl_easy_setopt with new proxy
    void setupProxy(CURL* curl, IP* proxy);

    // blacklisting proxy in db
    void badProxy(std::string& ip, mongocxx::collection& coll);

    // return IP obrect from DB. Contains IP, Port, Protocol, country, response time
    IP* getProxy(mongocxx::collection& coll);

    

public:

    CURLing();
    
    /// \brief Get all games from steam
    void GetAllGames();

    // Get URL contains
    void GetURL(const char*  link);

    // testing proxy change for 429 avoidance
    void ProxyTest(std::string& appid, mongocxx::collection& coll);
    
    // return cURL result
    std::string GetBuffer();
    
    // Encoding string to use in URL
    std::string url_encode(const std::string& str);
  
    ~CURLing() {};

};

