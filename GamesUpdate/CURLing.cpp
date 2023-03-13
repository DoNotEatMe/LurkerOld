#include "CURLing.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
//#include <windows.h>








CURLing::CURLing()
{

    curl = curl_easy_init();
    


}


void CURLing::GetAllGames()
{
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://api.steampowered.com/ISteamApps/GetAppList/v0002/?format=json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        //if (res != CURLE_OK) {
        //    printf( "Curl connection error" << std::endl;
        //}
        //else {
        //    // Request succeeded
        //    long response_code;
        //    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        //    if (response_code == 429) {
        //        printf( "Error 429: 'Too many requests'" << std::endl;
        //    }
        //    else {
        //        printf( "code: " << response_code << std::endl;
        //    }
        //}

    }
}

void CURLing::GetCurlFromJson(const char* link)
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
            
            log.HttpError("Curl connection Error: ", response_code);
            
            GetCurlFromJson(link);
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
                //readBuffer.clear();
                GetCurlFromJson(link);

            }

        }

        //curl_easy_cleanup(curl);
        
       
        
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

void CURLing::CheckCurl(CURLcode& res, const char* link) {

    
}

std::string CURLing::url_encode(const std::string& str)
{
    CURL* curl = curl_easy_init();
    char* encoded_str = curl_easy_escape(curl, str.c_str(), str.length());
    std::string result(encoded_str);
    curl_free(encoded_str);
    curl_easy_cleanup(curl);
    return result;
}




