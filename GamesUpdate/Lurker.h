#pragma once
#include <string>
#include <set>
#include "MongoDB.h"
#include <rapidjson/document.h>


class CURLing;


//Class for lurking Steam info
class Lurker : public MongoDB 
{
	
public:
	Lurker();
	Lurker(std::string uri);
	
	bsoncxx::types::b_date GetSeconds(time_t& now);

	//Force adding all games from all apps json ~30s
	void AllGamesInsertMany();

	//Update Game List ~2 min. require appid uniq index
	void UpdateGames();

	//Add game info with delay 1s. 200 requests per 5 min.
	void AddGameInfo();
	void AddGameInfo(std::vector<bsoncxx::document::value>& DocArr);
	//std::set<int> InsertedAppIds;
	std::vector<bsoncxx::document::value> DocArr;
	
	void AddReview();
	
	void CheckReview(int& app_id,  int& InsertedRecs,  int& RequestCount, mongocxx::collection& coll);
	
	
	
	

	~Lurker() {  };

private:

	 int InsertedGames = 0;
	 int InsertedRecs = 0;
	 int RequestCount = 0;
	 int JsonNotSuccess = 0;
	 int JsonNoReviews = 0;
	 int DupticatesInserted = 0;

	 int cached = 0;
	 void goCached();

	bsoncxx::types::b_date ConverttoISODate(int seconds);
	
	void InsertReviewByRecID( mongocxx::collection& coll, int& app_id, int64_t recID);

	void InsertReview(const rapidjson::Value& Doc, mongocxx::collection& coll, int i, int& app_id);

	bool isEqualTimestamp(int64_t TimeJson, int64_t TimeDB);

	bool isGreaterTimestamp(int64_t TimeJson, int64_t TimeDB);

	bool isTotalReviewsEqual(int& Json_total_reviews, mongocxx::collection& coll, int& app_id);
		
	void SetJson_total(const rapidjson::Value& Doc, std::vector <int64_t>& json_total);
	
	void SetDB_total(mongocxx::collection& coll, int& app_id, std::vector <int64_t>& db_total);

	void GetParsedJson(int& app_id, std::string cursor, rapidjson::Document& Doc, CURLing& curl);

	bool isSuccess(rapidjson::Document& Doc, int& app_id);

	bool isEmpty(rapidjson::Document& Doc, int& app_id);

	
	
	
};

