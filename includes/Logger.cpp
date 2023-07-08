#include "Logger.h"

#include <fstream>
#include <iomanip>
#include <sstream>

#include <filesystem>

#include <sstream>
#include <fstream>
#include <iomanip>

#include <iostream>

std::filesystem::path const Logger::logDir = std::filesystem::absolute("/root/projects/Lurker/log");

Logger::Logger() {

    

    if (std::filesystem::exists(logDir) && std::filesystem::is_directory(logDir)) {

    }
    else {
        try {
            std::filesystem::create_directories(logDir);
        }
        catch (const std::exception& ex) {
            std::cerr << "Failed to create log directory: " << ex.what() << std::endl;
        }
        
    }
    
    
}

//Unknown connection error
void Logger::Connection()
{
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $conn<<" << " " << "connection error" << std::endl;

    std::ofstream log( logDir / "connection_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}

//Connection error with msg
void Logger::Connection(std::string err)
{
       
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $conn<<" << " " << err << std::endl;

    
    std::ofstream log( logDir / "connection_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}

//Connection error with msg and code
void Logger::Connection(std::string err, std::string code)
{
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $conn<<" << " " << err << " " << code << std::endl;

    std::ofstream log( logDir / "connection_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}


//IsObject log check
void Logger::IsObject(int appid)
{
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $obj <<" << " " << "Appid:" << " " << appid << " " << "IsObject() == false" << std::endl;

    std::ofstream log( logDir / "insert_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}


//Appid + msg inserted
void Logger::InsertApp(int appid, std::string msg)
{    
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $add <<" << " " << "Appid:"<< " " << appid << " " << msg << std::endl;

    std::ofstream log(logDir / "insert_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}


//Insert games counter
void Logger::InsertCount(std::string msg, int GameCounter) {
   
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $add <<" << " " << msg << " " << GameCounter << std::endl;

    std::ofstream log(logDir / "insert_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();

}


//Inserted and Failed counter
void Logger::InsertCount(std::string msg,  int& Inserted,  int& Failed) {
    
    
    
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $add <<" << " " << msg << " " << Inserted << " " << "Inserted |" << " " << Failed << " " << "Failed" << std::endl;

    std::ofstream log(logDir / "insert_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}

void Logger::InsertCount(std::string games, std::string recs, int& Inserted,   int& Failed) {



    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $add <<" << " " << games << " " << Inserted << " " << "Inserted |" << " " << recs << " " << Failed << " Inserted" << std::endl;

    std::ofstream log(logDir / "insert_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}



//HTTP error msg and code
void Logger::HttpError(std::string err, long int& code)
{
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $http<<" << " " << err << " " << code << std::endl;

    std::ofstream log(logDir / "httpError_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
}

//HTTP error msg
void Logger::HttpError(std::string err)
{
        
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $http<<" << " " << err << std::endl;

    std::ofstream log(logDir / "httpError_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();
    
    
}


void Logger::Status(std::string msg) {
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $sys <<" << " " << msg << std::endl;

    std::ofstream log(logDir / "status_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();

}

void Logger::Status(std::string msg,   int count) {
    std::ostringstream out;
    out << ConvertTime(GetTime()) << " $sys <<" << " " << msg << " " << count << std::endl;

    std::ofstream log(logDir / "status_log", std::ios::app);
    log << out.str();

    GlobalLog(out.str());

    log.close();

}



void Logger::GlobalLog(const std::string& content) {
    
   std::ofstream global_log(logDir / "global_log", std::ios::app);
   
   global_log << content;
   
   global_log.close();

   LurkGlobalLog(content);
}


void Logger::LurkGlobalLog(const std::string& content) {

    std::ofstream Lurkglobal_log("/root/projects/Lurker/LurkGlobal_log", std::ios::app);

    Lurkglobal_log << content;

    Lurkglobal_log.close();

}



std::string Logger::ConvertTime(time_t now)
{
    struct tm t;
    localtime_r(&now , &t);

    std::ostringstream oss;
    oss << std::put_time(&t, "%d/%m/%y %H:%M:%S");
    auto str = oss.str();


    return str;
}

time_t Logger::GetTime() {
    
   time_t now = time(nullptr);;
   return now;
}


void Logger::ExecutionTime(std::string msg) {
    auto end_time = std::chrono::steady_clock::now();

    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double duration_s = static_cast<double>(duration_ms.count()) / 1000;

    
    int sectoint = duration_s;
        
    int mins = sectoint / 60;
    int secs = sectoint % 60;
    int hours = mins / 60;
    int days = hours / 24;

    std::ostringstream out;

    if(mins >= 1 && hours < 1){

        out << ConvertTime(GetTime()) << " $sys <<" << " " << msg << " Execution time " << mins << ":" << secs << std::endl;
    }
    
    if (mins > 1 && hours > 1){
        
        out << ConvertTime(GetTime()) << " $sys <<" << " " << msg << " Execution time " << mins%60 << ":" << mins << ":" << secs << std::endl;
    }
    if (mins < 1 && hours < 1) {
        
        out << ConvertTime(GetTime()) << " $sys <<" << " " << msg << " Execution time " << secs << "s" << std::endl;
    }
    


    GlobalLog(out.str());
    
}

void Logger::SetExecutionTimer() {

    start_time = std::chrono::steady_clock::now();
}

void Logger::aps(int actions) {

    auto end_time = std::chrono::steady_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double duration_s = static_cast<double>(duration_ms.count()) / 1000;

    std::ostringstream out;

    out << ConvertTime(GetTime()) << " $sys <<" << " " << "avg action time " << duration_s / actions << "s" << std::endl;
    
    GlobalLog(out.str());
}


