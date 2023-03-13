#pragma once

#include <string>
#include <bsoncxx/types.hpp>
#include <filesystem>

class Logger
{

private:


	time_t GetTime();

	std::chrono::steady_clock::time_point start_time;

	std::string ConvertTime(time_t now);

	void GlobalLog(const std::string& content);
	void LurkGlobalLog(const std::string& content);

	

public:
	Logger();

	
	static std::filesystem::path const logDir;
	
	//Unknown connection error
	void Connection();
	//Connection error with msg
	void Connection(std::string err);
	//Connection error with msg and code
	void Connection(std::string err, std::string code);

	//IsObject log check
	void IsObject(int appid);
	//Appid + msg inserted
	void InsertApp(int appid, std::string msg);

	//Insert games counter
	void InsertCount(std::string msg, int GameCounter);
	//Inserted and Failed counter
	void InsertCount(std::string msg,  int& Inserted,  int& Failed);



	void InsertCount(std::string games, std::string recs,  int& Inserted,   int& Failed);

	//HTTP error msg and code
	void HttpError(std::string err, long int& code);
	//HTTP error msg
	void HttpError(std::string err);


	//Program Status message
	void Status(std::string msg);
	void Status(std::string msg,   int count);

	//Execution time
	void ExecutionTime(std::string msg);

	void aps(int actions);

	//Set Timer for execution
	void SetExecutionTimer();
	

	
	



};

