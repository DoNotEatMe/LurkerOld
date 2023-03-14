// ProxyHolder.cpp : Defines the entry point for the application.
//

#include <set>

#include "ProxyHolder.h"
#include "CURLing.h"
#include "MongoDB.h"
#include "Logger.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>

#include <mongocxx/exception/bulk_write_exception.hpp>


#include "rapidjson/document.h"

bsoncxx::types::b_date GetSeconds();
bsoncxx::types::b_date ConverttoISODate(int seconds);

int main()
{

	Logger log;
	log.SetExecutionTimer();
	log.Status("ProxyHolder() starter");

	CURLing curl;

	curl.GetCurlFromJson("https://proxylist.geonode.com/api/proxy-list?limit=500&page=1&sort_by=lastChecked&sort_type=desc");
	//std::cout << curl.GetBuffer() << std::endl;

	rapidjson::Document RapidDoc;
	RapidDoc.Parse(curl.GetBuffer().c_str());

	rapidjson::Value& Data = RapidDoc["data"];

	if (!RapidDoc.IsObject()) {
		log.Status("ProxyHolder() Object Assertion failed;");
 		return 1;
	}

	MongoDB db;
	db.SetDatabaseName("steam");
	db.SetCollectionName("proxylist");
	auto coll = db.GetClient()->database(db.GetDatabaseName()).collection(db.GetCollectionName());

	std::vector <bsoncxx::document::view_or_value> ProxyList;

	
	for (rapidjson::SizeType i = 0; i < Data.Size(); i++) {
	
		ProxyList.push_back(
			bsoncxx::builder::stream::document{}
			<< "ip" << Data[i]["ip"].GetString()
			<< "country" << Data[i]["country"].GetString()
			<< "city" << Data[i]["city"].GetString()
			<< "port" << Data[i]["port"].GetString()
			<< "lastChecked" << ConverttoISODate(Data[i]["lastChecked"].GetInt())
			<< bsoncxx::builder::stream::finalize
		);

	}



	mongocxx::bulk_write bulk_ops = coll.create_bulk_write();

	for (const auto& doc : ProxyList) {
		

		mongocxx::model::update_one update_one{ 
							bsoncxx::builder::stream::document{} 
								<< "ip" << doc.view()["ip"].get_string().value 
								<< bsoncxx::builder::stream::finalize,
							bsoncxx::builder::stream::document {}
								<< "$set"
								<< bsoncxx::builder::stream::open_document
								<< bsoncxx::builder::concatenate(doc.view())
								<< "serverupdated" << GetSeconds()
								<< bsoncxx::builder::stream::close_document
								<< bsoncxx::builder::stream::finalize								
							 };

		update_one.upsert(true);

		try {
			bulk_ops.append(update_one);
		}
		catch (const mongocxx::bulk_write_exception& e) {
			//std::cout << "Bulk write exception occurred: " << e.what() << std::endl;
			std::string err = e.what();
			log.Status("Bulk write exception occurred: " + err);
			log.InsertApp(000000, "Failed with doc " + bsoncxx::to_json(doc.view()));
			
		}
	}

	bulk_ops.execute();
	
	
	auto cursor = coll.find(
		bsoncxx::builder::stream::document{}
		<< "works" << false
		<< bsoncxx::builder::stream::finalize);

	if (cursor.begin() != cursor.end()){
	
		mongocxx::bulk_write bulk_works = coll.create_bulk_write();
		for (auto& doc : cursor) {
		
			mongocxx::model::update_one update{ 
				bsoncxx::builder::stream::document{}
					<< "ip" << doc["ip"].get_string().value
					<< bsoncxx::builder::stream::finalize,
				bsoncxx::builder::stream::document{}
					<< "$set"
					<< bsoncxx::builder::stream::open_document
					<< "works" << true
					<< bsoncxx::builder::stream::close_document
					<< bsoncxx::builder::stream::finalize
			};
			update.upsert(true);
			bulk_works.append(update);
		}
		bulk_works.execute();
	}

	log.Status("ProxyHolder() ended");
	log.ExecutionTime("ProxyHolder()");

	return 0;
}

bsoncxx::types::b_date GetSeconds() {

	time_t now = time(nullptr);
	auto bson_date = bsoncxx::types::b_date{ std::chrono::milliseconds{now * 1000} };

	return bson_date;
}

bsoncxx::types::b_date ConverttoISODate(int seconds) {

	auto time_in_seconds = std::chrono::seconds(seconds);
	bsoncxx::types::b_date ISODate(bsoncxx::types::b_date{ time_in_seconds });

	return ISODate;

}