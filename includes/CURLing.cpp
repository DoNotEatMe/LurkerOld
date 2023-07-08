#include "CURLing.h"
#include <fstream>
#include <iostream>
#include <unistd.h>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/string_view.hpp>

CURLing::CURLing()
{
    curl = curl_easy_init();
}

void CURLing::GetAllGames()
{
    readBuffer.clear();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://api.steampowered.com/ISteamApps/GetAppList/v0002/?format=json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
}

void CURLing::GetURL(const char* link)
{
    readBuffer.clear();

    if (curl) {
        
        curl_easy_setopt(curl, CURLOPT_URL, link);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
   
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (res != CURLE_OK) {
            std::string error = curl_easy_strerror(res);
            log.HttpError("Curl connection Error: ", response_code);
            log.HttpError("Curl connection Error: " + error);
            
            GetURL(link);
        }
        else {
            if (response_code == 429) {

                log.HttpError("Error 429: 'Too many requests");

                int awake = 0;
                while (awake != 1) {
                    log.HttpError("Sleeping for 1 minute");
                    int awake_in = 1;
                    sleep(60);                    
                    awake++;
                }
             
                log.HttpError("Awaked. Oh shit, here we go again");
                GetURL(link);
            }
        }

        curl_easy_cleanup(curl);

    }
}

size_t CURLing::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string CURLing::GetBuffer() {
    return this->readBuffer;
}

std::string CURLing::url_encode(const std::string& str)
{
    CURL* curl = curl_easy_init();
    char* encoded_str = curl_easy_escape(curl, str.c_str(), str.length());
    std::string result(encoded_str);
    curl_free(encoded_str);
    return result;
}

IP* CURLing::getProxy(mongocxx::collection& coll) {

    //Add response time to get best
    bsoncxx::builder::stream::document SearchIP;
    SearchIP
        << "works" << bsoncxx::builder::stream::open_document
        << "$ne" << false << bsoncxx::builder::stream::close_document
        << "protocols" << bsoncxx::builder::stream::open_document
        << "$ne" << "socks4" << bsoncxx::builder::stream::close_document;
    
    auto cursor = coll.find_one(SearchIP.view());

    //db.~MongoDB();
    if (cursor.has_value()) {
        IP* obj = new IP(cursor.value().view());
        return obj;
    }

    
    return nullptr;
}

void CURLing::setupProxy(CURL* curl, IP* proxy) {
    std::string proxyStr = proxy->getProtocol() + "://" + proxy->getIP() + ":" + proxy->getPort();
    //std::string proxyStr = "212.102.54.28:3128";
    curl_easy_setopt(curl, CURLOPT_PROXY, proxyStr);
    //curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    std::cout << proxyStr << std::endl;

}

void CURLing::ProxyTest(std::string& appid, mongocxx::collection& coll) {
    
    readBuffer.clear();

    if (curl) {
        
        if (ip.empty()) {
            IP* ptr = getProxy(coll);
            setupProxy(curl, ptr);
            ip = ptr->getIP();
            delete ptr;
        }
        
        std::string link = "https://store.steampowered.com/api/appdetails?appids=" + appid;

        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {

            std::string error = curl_easy_strerror(res);
            
            log.HttpError("Curl connection Error: ", response_code);
            log.HttpError("Curl connection Error: " + error);

            badProxy(ip, coll);
            ip.clear();
            log.HttpError("changing proxy");
            ProxyTest(appid, coll);

        }
        if (response_code == 429) {
            log.HttpError("Error 429: 'Too many requests");

            badProxy(ip, coll);
            ip.clear();
            log.HttpError("changing proxy");
            ProxyTest(appid, coll);
        }
        
    }
}

void CURLing::badProxy(std::string& ip, mongocxx::collection& coll) {

    

    mongocxx::model::update_one update{
                bsoncxx::builder::stream::document{}
                    << "ip" << ip
                    << bsoncxx::builder::stream::finalize,
                bsoncxx::builder::stream::document{}
                    << "$set"
                    << bsoncxx::builder::stream::open_document
                    << "works" << false
                    << bsoncxx::builder::stream::close_document
                    << bsoncxx::builder::stream::finalize
    };

    update.upsert(true);

    coll.update_one(update.filter(), update.update());

    std::cout << ip << " blacklisted" << std::endl;
}

