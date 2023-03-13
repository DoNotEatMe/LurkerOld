// ProxyHolder.cpp : Defines the entry point for the application.
//

#include "ProxyHolder.h"

#include "Logger.h"

#include <curl/curl.h>

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
void GetCurlFromJson(std::string& ProxyList, std::string& readBuffer, CURL* curl, CURLcode& res, Logger& log);

int main()
{
    Logger log;

	CURL* curl;
	CURLcode res;

	std::string readBuffer;
	
    std::string ProxyList = "https://proxylist.geonode.com/api/proxy-list?limit=500&page=1&sort_by=lastChecked&sort_type=desc";

    GetCurlFromJson(ProxyList, readBuffer, curl, res, log);

    curl_easy_cleanup(curl);


    std::cout << readBuffer << std::endl;


	
	return 0;
}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void GetCurlFromJson(std::string& ProxyList, std::string& readBuffer, CURL* curl, CURLcode& res, Logger& log) {

    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, ProxyList);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (res != CURLE_OK) {

            log.HttpError("Curl connection Error: ", response_code);

            GetCurlFromJson(ProxyList, readBuffer, curl, res, log);
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
                GetCurlFromJson(ProxyList, readBuffer, curl, res, log);

            }

        }
    }
}
