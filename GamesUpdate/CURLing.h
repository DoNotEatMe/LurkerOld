#pragma once
#include <string>

#include <curl/curl.h>
#include "Logger.h"


/*
CURL Setup for Json record
*/
class CURLing {
    

private:
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);


    
    
    Logger log;

public:

    CURLing();
    void GetAllGames();
    void GetCurlFromJson(const char*  linktoparse);
    void CheckCurl(CURLcode& res, const char* link);
    std::string GetBuffer();
    std::string NullStr = "Null";
    
    std::string url_encode(const std::string& str);
  
    

    ~CURLing() { curl_easy_cleanup(curl); };

};