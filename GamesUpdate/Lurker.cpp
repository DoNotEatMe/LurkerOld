#include "Lurker.h"
#include <iostream>

//mongocxx

#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/query_exception.hpp>

#include <mongocxx/pipeline.hpp>
#include <mongocxx/stdx.hpp>

#include <mongocxx/bulk_write.hpp>

//bosncxx
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>



//rapidjson
//#include <rapidjson/document.h>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

//Custom classes
#include "CURLing.h"
#include "Game.h"
#include "Logger.h"

//std
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include <thread>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using rapidjson::Document;

std::filesystem::path const Logger::logDir = std::filesystem::absolute("/root/projects/Lurker/" + std::string(PROJECT_NAME) + "/log");

//Create connection to localhost without password
Lurker::Lurker() {
    
}


//Create connection to URL without password
Lurker::Lurker(std::string uri) {
    
}

//Force adding all games from all apps json ~30s
void Lurker::AllGamesInsertMany() {

    log.SetExecutionTimer();
    log.Status("AllGamesInsertMany() started");

    CURLing curl;
    curl.GetAllGames(); 
    
    //change DB names if neccesary with setters
    auto coll = GetClient()->database(GetDatabaseName()).collection(GetCollectionName());
     
    rapidjson::Document document;
    document.Parse(curl.GetBuffer().c_str());

    const rapidjson::Value& apps = document["applist"]["apps"];

    std::vector<bsoncxx::document::value> DocArr;

    int GamesCount(0);

    for (rapidjson::SizeType i = 0; i < apps.Size(); i++) {
        Game* buff = new Game(apps[i]["appid"].GetInt(), apps[i]["name"].GetString());
        DocArr.push_back(buff->doc());
        delete buff;
        GamesCount++;
    }

    mongocxx::options::insert opts;
    opts.ordered(false);

    // Create an index on the "myfield" field with ascending order
    bsoncxx::builder::basic::document index{};
    index.append(bsoncxx::builder::basic::kvp("appid", 1));

    // Create the index
    mongocxx::options::index options{};
    options.background(true);
    options.unique(true);
    coll.create_index(index.view(), options);

    try {
        coll.insert_many(DocArr, opts);
        DocArr.clear();
    }
    catch (mongocxx::bulk_write_exception& e) {
        std::cout << "code=" << e.code() << "\n";
        std::cout << "message=" << e.what(), "\n";
        if (e.raw_server_error()) {
            std::cout << "error reply=" << bsoncxx::to_json(*e.raw_server_error()) << "\n";
        }
    }

    log.InsertCount("AllGamesInsertMany() Games inserted: ", GamesCount);
    log.Status("AllGamesInsertMany() ended");
    log.ExecutionTime("AllGamesInsertMany()");
    
}

//check if game exist in DB, add if not add new ~1s
void Lurker::UpdateGames()
{
    // log setup
    log.SetExecutionTimer();
    log.Status("UpdateGames() started");

    // CURL and DB connection
    CURLing curl;
    curl.GetAllGames();
    auto coll = GetClient()->database(GetDatabaseName()).collection(GetCollectionName());

    // JSON parse
    rapidjson::Document document;
    document.Parse(curl.GetBuffer().c_str());

    if (!document.IsObject()) {
        log.Status("Parsing error. Document IsObject: False");
    }

    const rapidjson::Value& apps = document["applist"]["apps"];

    // count setup
    int Inserted(0);
    int Failed(0);

    // Looking for all Appids in DB and store them in set
    auto GetAllAppid = mongocxx::pipeline{};
    GetAllAppid.append_stage(
         make_document(kvp("$project", make_document(
             kvp("_id", 0),
             kvp("appid", 1)
         ))));

    auto AgregatedFind = coll.aggregate(GetAllAppid);

    std::set<int> foundAppIds;

    for (auto&& Find : AgregatedFind) {
        foundAppIds.insert(Find["appid"].get_int32());
    }


    // Get each Appid from parsed JSON and insert into DocArr if not exist
    for (rapidjson::SizeType i = 0; i < apps.Size(); i++) {
 
        int appId = apps[i]["appid"].GetInt();

        if (foundAppIds.count(appId) > 0) {
            Failed++;
            continue;
        }
        else {
    
            DocArr.push_back(bsoncxx::builder::stream::document{}
                    << "appid" << appId
                    << "name" << apps[i]["name"].GetString()
                    << bsoncxx::builder::stream::finalize);

            //log.InsertApp(appId, "added");
            Inserted++;
        }
    }

    log.InsertCount("UpdateGames()", Inserted, Failed );
    log.Status("UpdateGames() ended");
    log.ExecutionTime("UpdateGames()");
}

//Add game info with delay 1s. 200 requests per 5 min.
void Lurker::AddGameInfo() {
    log.SetExecutionTimer();
    log.Status("AddGameInfo() started");

    auto coll = GetClient()->database(GetDatabaseName()).collection(GetCollectionName());
  
    /* bsoncxx::builder::stream::document filter_builder;
     filter_builder << "appid" << 1056250;
      auto cursor = coll.find( filter_builder.view() );*/

    auto cursor = coll.find({});


    
     int Inserted(0);
     int Failed(0);

    //loop for adding for find results
    for (auto&& doc : cursor) {

        auto it = doc.find("last_update");
        if (it != doc.end()) { continue; }

        

        time_t now = time(nullptr);
        
        std::chrono::seconds duration(1);
        std::this_thread::sleep_for(duration);


        CURLing curl;

        bsoncxx::document::view view = doc;
        auto app_id = view["appid"].get_int32();
        std::string appid_string = std::to_string(app_id);
        std::string SteamURL_sting = "https://store.steampowered.com/api/appdetails?appids=" + appid_string;
        curl.GetCurlFromJson(SteamURL_sting.c_str());

        //reading json
        rapidjson::Document document;
        document.Parse(curl.GetBuffer().c_str());

        if (!document.IsObject()) {
            log.IsObject(app_id);
            Failed++;
            continue;
        }

        const rapidjson::Value& appscheck = document[appid_string.c_str()];
        const rapidjson::Value& success = appscheck["success"];

        if (success.GetBool() == false) {

            bsoncxx::builder::stream::document update_builder;
            update_builder << "$set" << bsoncxx::builder::stream::open_document
                << "last_update" << GetSeconds(now)
                << bsoncxx::builder::stream::close_document;

            auto update_result = coll.update_one(
                doc,
                update_builder.view());

            Failed++;
            log.InsertApp(app_id, "skipped");
            continue;
        }

        const rapidjson::Value& apps = appscheck["data"];

        bsoncxx::builder::stream::document update_builder;
        update_builder << "$set" << bsoncxx::builder::stream::open_document
            << "type" << apps["type"].GetString()
            << "coming_soon" << apps["release_date"]["coming_soon"].GetBool()
            << "url" << apps["support_info"]["url"].GetString()
            << "email" << apps["support_info"]["email"].GetString()
            << "is_free" << apps["is_free"].GetBool()
            << "last_update" << GetSeconds(now)
            << bsoncxx::builder::stream::close_document;

        auto update_result = coll.update_one(
            doc,
            update_builder.view()
        );

        if (apps.HasMember("packages")) {

            bsoncxx::builder::basic::array packages_array;
            for (const auto& packages : apps["packages"].GetArray()) {
                packages_array.append(bsoncxx::types::b_int32{ packages.GetInt() });
            }

            bsoncxx::builder::stream::document update_builder;
            update_builder << "$set" << bsoncxx::builder::stream::open_document
                << "packages" << packages_array
                << bsoncxx::builder::stream::close_document;
            auto update_result = coll.update_one(
                doc,
                update_builder.view()
            );
        }

        if (apps.HasMember("recommendations")) {
            bsoncxx::builder::stream::document update_builder;
            update_builder << "$set" << bsoncxx::builder::stream::open_document
                << "recommendations" << apps["recommendations"]["total"].GetInt()
                << bsoncxx::builder::stream::close_document;
            auto update_result = coll.update_one(
                doc,
                update_builder.view()
            );
        }

        if (apps.HasMember("fullgame")) {

            int num = std::stoi(apps["fullgame"]["appid"].GetString());

            update_builder << "$set" << bsoncxx::builder::stream::open_document
                << "parrent" << num
                << bsoncxx::builder::stream::close_document;

            auto update_result = coll.update_one(
                doc,
                update_builder.view()
            );


        }

        if (apps.HasMember("developers")) {

            bsoncxx::builder::basic::array developers_array;
            for (const auto& developer : apps["developers"].GetArray()) {
                developers_array.append(bsoncxx::types::b_utf8{ developer.GetString() });
            }

            bsoncxx::builder::stream::document update_builder;
            update_builder << "$set" << bsoncxx::builder::stream::open_document
                << "developers" << developers_array
                << bsoncxx::builder::stream::close_document;

            auto update_result = coll.update_one(
                doc,
                update_builder.view()
            );

        }

        if (apps.HasMember("publishers")) {
            bsoncxx::builder::basic::array publishers_array;
            for (const auto& publishers : apps["publishers"].GetArray()) {
                publishers_array.append(bsoncxx::types::b_utf8{ publishers.GetString() });
            }

            bsoncxx::builder::stream::document update_builder;
            update_builder << "$set" << bsoncxx::builder::stream::open_document
                << "publishers" << publishers_array
                << bsoncxx::builder::stream::close_document;

            auto update_result = coll.update_one(
                doc,
                update_builder.view());

        }

        if (apps.HasMember("release_date") && apps["release_date"].HasMember("date")) {


            //497320 release_date = "Coming Soon"

            if (apps["release_date"]["date"].GetStringLength() != 0) {

                std::string date_str = apps["release_date"]["date"].GetString();


                std::tm t = {};
                std::istringstream ss(date_str);
                if (ss >> std::get_time(&t, "%d %b, %Y")) {

                    std::ostringstream os;
                    os << std::put_time(&t, "%Y-%m-%dT%H:%M:%SZ");


                }
                else {
                    //printf( "Error: First format failed" << std::endl;
                    ss.clear(); // clear the fail bit
                    ss.seekg(0, ss.beg); // reset the input stream
                    if (ss >> std::get_time(&t, "%b %Y")) {
                        std::ostringstream os;
                        t.tm_mday = 1;
                        os << std::put_time(&t, "%Y-%m-%dT%H:%M:%SZ");



                    }
                }




                time_t seconds = std::mktime(&t);
                
                
                //TODO: Refactor to ConverttoISODate
                auto time_in_seconds = std::chrono::seconds(seconds);

                bsoncxx::types::b_date release_date(bsoncxx::types::b_date{ time_in_seconds });


                bsoncxx::builder::stream::document update_builder;
                update_builder << "$set" << bsoncxx::builder::stream::open_document
                    << "release_date" << release_date
                    << bsoncxx::builder::stream::close_document;

                auto update_result = coll.update_one(
                    doc,
                    update_builder.view()
                );
            }
        }

        Inserted++;
    }
    
    log.InsertCount("AddGameInfo() done", Inserted, Failed);
    log.Status("AddGameInfo() ended");
    log.ExecutionTime("AddGameInfo()");
}


// DocArr with games that not in DB
void Lurker::AddGameInfo(std::vector<bsoncxx::document::value>& DocArr) {

    // starting log setup
    log.SetExecutionTimer();
    log.Status("AddGameInfo() started");

    // database connection
    auto coll = GetClient()->database(GetDatabaseName()).collection(GetCollectionName());

    // counting setup
    int Inserted(0);
    int Failed(0);
    

    // adding each game from vector
    for (auto&& doc : DocArr) {

        Inserted++;
        time_t now = time(nullptr);

        std::chrono::seconds duration(1);
        std::this_thread::sleep_for(duration);

        // setup curl and get json
        CURLing curl;
        std::string appid_string = std::to_string(doc["appid"].get_int32());
        std::string SteamURL_sting = "https://store.steampowered.com/api/appdetails?appids=" + appid_string;
        curl.GetCurlFromJson(SteamURL_sting.c_str());

        // Parsing json
        rapidjson::Document document;
        document.Parse(curl.GetBuffer().c_str());

        // check if is json
        if (!document.IsObject()) {
            log.IsObject(doc["appid"].get_int32());
            Failed++;
            goCached();
            continue;
        }

        const rapidjson::Value& appscheck = document[appid_string.c_str()];
        const rapidjson::Value& success = appscheck["success"];

        // if response is false
        if (success.GetBool() == false) {

            bsoncxx::builder::stream::document builder{};
            builder << bsoncxx::builder::concatenate(doc.view())
                    << "last_update" << GetSeconds(now);
                    
            doc = builder
                    << bsoncxx::builder::stream::finalize;

            builder.clear();
            Failed++;
            log.InsertApp(doc["appid"].get_int32(), "Success.GetBool() == false");
            goCached();
            continue;
        }

        const rapidjson::Value& apps = appscheck["data"];

        bsoncxx::builder::stream::document HasPackages{};
        if (apps.HasMember("packages")) {

            bsoncxx::builder::basic::array packages_array;
            for (const auto& packages : apps["packages"].GetArray()) {
                packages_array.append(bsoncxx::types::b_int32{ packages.GetInt() });
            }
            
            HasPackages
                << "packages" << packages_array; 
        }
        
        bsoncxx::builder::stream::document HasReccomendations{};
        if (apps.HasMember("recommendations")) {
                       
            bsoncxx::builder::stream::document builder{};
            HasReccomendations
                << "recommendations" << apps["recommendations"]["total"].GetInt();

        }
        
        bsoncxx::builder::stream::document HasFullgame{};
        if (apps.HasMember("fullgame")) {

            int num = std::stoi(apps["fullgame"]["appid"].GetString());

            bsoncxx::builder::stream::document builder{};
            HasFullgame
                << "parrent" << num;
            
        }
        
        bsoncxx::builder::stream::document HasDevelopers{};
        if (apps.HasMember("developers")) {

            bsoncxx::builder::basic::array developers_array;
            for (const auto& developer : apps["developers"].GetArray()) {
                developers_array.append(bsoncxx::types::b_utf8{ developer.GetString() });
            }

            HasDevelopers
                << "developers" << developers_array;
        }

        bsoncxx::builder::stream::document HasPublishers{};
        if (apps.HasMember("publishers")) {

            bsoncxx::builder::basic::array publishers_array;
            for (const auto& publishers : apps["publishers"].GetArray()) {
                publishers_array.append(bsoncxx::types::b_utf8{ publishers.GetString() });
            }

            HasPublishers
                << "publishers" << publishers_array;
            
        }
        
        bsoncxx::builder::stream::document Hasrelease_date{};
        if (apps.HasMember("release_date") && apps["release_date"].HasMember("date")) {

            if (apps["release_date"]["date"].GetStringLength() != 0) {

                std::string date_str = apps["release_date"]["date"].GetString();

                std::tm t = {};
                std::istringstream ss(date_str);

                if (ss >> std::get_time(&t, "%d %b, %Y")) {
                    std::ostringstream os;
                    os << std::put_time(&t, "%Y-%m-%dT%H:%M:%SZ");
                }
                else {
                    //printf( "Error: First format failed" << std::endl;
                    ss.clear(); // clear the fail bit
                    ss.seekg(0, ss.beg); // reset the input stream
                    if (ss >> std::get_time(&t, "%b %Y")) {
                        std::ostringstream os;
                        t.tm_mday = 1;
                        os << std::put_time(&t, "%Y-%m-%dT%H:%M:%SZ");
                    }
                }

                time_t seconds = std::mktime(&t);

                auto time_in_seconds = std::chrono::seconds(seconds);

                bsoncxx::types::b_date release_date(bsoncxx::types::b_date{ time_in_seconds });

                Hasrelease_date
                    << "release_date" << release_date;
                
            }
        }
        
        // Doc former
        bsoncxx::builder::stream::document builder{};
        
        builder
            << bsoncxx::builder::concatenate(doc.view())
            << "type" << apps["type"].GetString()
            << "coming_soon" << apps["release_date"]["coming_soon"].GetBool()
            << "url" << apps["support_info"]["url"].GetString()
            << "email" << apps["support_info"]["email"].GetString()
            << "is_free" << apps["is_free"].GetBool()
            << "last_update" << GetSeconds(now)
            << bsoncxx::builder::concatenate(HasPackages.view())
            << bsoncxx::builder::concatenate(HasReccomendations.view())
            << bsoncxx::builder::concatenate(HasFullgame.view())
            << bsoncxx::builder::concatenate(HasDevelopers.view())
            << bsoncxx::builder::concatenate(HasPublishers.view())
            << bsoncxx::builder::concatenate(Hasrelease_date.view());
        doc = builder
            << bsoncxx::builder::stream::finalize;

        builder.clear();

        goCached();

       
        
    }
    

    // Update bulk
    mongocxx::bulk_write bulk_ops = coll.create_bulk_write();
        
    for (const auto& doc : DocArr) {
        
        mongocxx::model::insert_one insert_op{ doc.view() };
        
        try{
            bulk_ops.append(insert_op);
        }
        catch (const mongocxx::bulk_write_exception& e) {
            std::cout << "Bulk write exception occurred: " << e.what() << std::endl;
            log.InsertApp(000000, "Failed with doc " + bsoncxx::to_json(doc.view()));
            Failed++;
        }
    }

    bulk_ops.execute();

    int succeed(0);
    (Failed > 0) ? succeed = (Inserted - Failed) : succeed = Inserted;

    log.InsertCount("AddGameInfo() done", succeed, Failed);
    log.Status("AddGameInfo() ended");
    log.ExecutionTime("AddGameInfo()");
    log.aps(Inserted);
}

bsoncxx::types::b_date Lurker::GetSeconds(time_t& now) {


    auto bson_date = bsoncxx::types::b_date{ std::chrono::milliseconds{now * 1000} };

    return bson_date;
}

//Collect Review

void Lurker::AddReview() {
    
    //Log start
    log.SetExecutionTimer();
    log.Status("AddReview() started");

    //counting
    

    //Connect to Reviews
    auto DBcollection = GetClient()->database(GetDatabaseName()).collection(GetCollectionName());

    //TODO: ??
    //time_t TimeTNow = time(nullptr);
        

    //Indexing
    bsoncxx::builder::basic::document indexAppid{};
    indexAppid.append(bsoncxx::builder::basic::kvp("appid", 1));
    bsoncxx::builder::basic::document indexRecid{};
    indexRecid.append(bsoncxx::builder::basic::kvp("recommendationid", 1));
   
    mongocxx::options::index optionsAppid{};
    optionsAppid.background(true);
    mongocxx::options::index optionsRecid{};
    optionsRecid.background(true);
    optionsRecid.unique(true);
    
    DBcollection.create_index(indexAppid.view(), optionsAppid);
    DBcollection.create_index(indexRecid.view(), optionsRecid);
    
    //Connect to Games
    auto DBcollectionGames = GetClient()->database("test").collection("games");
    
    //Filter to find games between release time
    auto nowf = std::chrono::system_clock::now();
    auto start_date = bsoncxx::types::b_date{ std::chrono::system_clock::from_time_t(1672572300) }; // January 1, 2023
    auto end_date = bsoncxx::types::b_date{ nowf };


    //TODO: Сделать аггрегацию по типу - игра.
    auto filter = bsoncxx::builder::stream::document{}
        << "release_date" << bsoncxx::builder::stream::open_document
        << "$gte" << start_date
        << "$lte" << end_date
        << bsoncxx::builder::stream::close_document
        << "type" << "game"     
        << bsoncxx::builder::stream::finalize;

    auto FindGames = DBcollectionGames.find(filter.view());

    //auto count = DBcollectionGames.count_documents(filter.view());
    //printf( "Number of documents: " << count << std::endl;
  
    //Main loop for the filter
    for (auto&& Game:FindGames) {
     

        //URL setup
        auto app_id = Game["appid"].get_int32().value;
        //auto app_id = 990080;
       
        
        CheckReview(app_id, InsertedRecs, RequestCount, DBcollection);
        InsertedGames++;
        //TODO: Update game info?
    }

    
    log.InsertCount("Games inserted:", InsertedGames);
    log.InsertCount("Reviews inserted:", InsertedRecs);
    log.InsertCount("Api requests done:", RequestCount);
    log.InsertCount("Json return success != 1:", JsonNotSuccess);
    log.InsertCount("Json no reviews:", JsonNoReviews);
    log.InsertCount("Rewrited and added reviews in missed games:", DupticatesInserted);
    log.ExecutionTime("AddReview()");
    log.Status("AddReview() ended");

}


bsoncxx::types::b_date Lurker::ConverttoISODate(int seconds){

    auto time_in_seconds = std::chrono::seconds(seconds);
    bsoncxx::types::b_date ISODate(bsoncxx::types::b_date{ time_in_seconds });

    return ISODate;

}

void Lurker::CheckReview(int& app_id,  int& InsertedRecs,  int& RequestCount, mongocxx::collection& coll) {

    
    std::string cursor = "*";
    
    rapidjson::Document document;
    CURLing curl;

    GetParsedJson(app_id, cursor,  document, curl);
    if (!isSuccess(document, app_id) || isEmpty(document, app_id)) { return; }
    const rapidjson::Value& reviews = document["reviews"];
    int Json_total_reviews = document["query_summary"]["total_reviews"].GetInt();
        
    //Looking for latest timestamp for appid
    auto pipelineB = mongocxx::pipeline{};
    pipelineB.append_stage(
        make_document(kvp("$match", make_document(
            kvp("appid", app_id)
        ))));
                
    pipelineB.append_stage(
        make_document(
            kvp("$group", make_document(
                kvp("_id", "$appid"),
            kvp("latest_timestamp", make_document(
                kvp("$max", "$timestamp_created")
            ))))));

    auto ReviewsAggregationB = coll.aggregate(pipelineB);

    int64_t TimeDB(0);
    for (auto&& Game : ReviewsAggregationB) {
        bsoncxx::types::b_date latest_timestamp = Game["latest_timestamp"].get_date();
        TimeDB = latest_timestamp.to_int64();
        TimeDB /= 1000;
    }
   
    
    //Если время создания первого отзыва из JSON и максимальное время из базы данных не равно
    if (!isEqualTimestamp(reviews[0]["timestamp_created"].GetInt(), TimeDB))
    {
        // выолняем пока не порвем цикл. Цикл нужен для переключения страниц по курсору
        while (true)
        {
            const rapidjson::Value& reviews = document["reviews"];

            bool isFinished = false;
            //Берем текущий JSON
            for (rapidjson::SizeType i = 0; i < reviews.Size(); i++)
            {
                //Если время создания отзыва в итерации i больше чем максимальное время из бд
                if (isGreaterTimestamp(reviews[i]["timestamp_created"].GetInt(), TimeDB))
                {
                    //printf( reviews[i]["timestamp_created"].GetInt() << " > " << TimeDB << std::endl;
                    //вставляем этот отзыв
                    InsertReview(reviews, coll, i, app_id);
                    InsertedRecs++;
                }
                else {
                    //если нет, значит мы дошли до равенства.
                    //Но стоит предусматривать то что в середине может быть дырка и бд дата будет больше чем то что должно быть вставлено
                    //printf( "break" << std::endl;
                    isFinished = true;
                    break;
                }
            }

            if (isFinished) { break; }

            //Контроллер курсора == выхода из цикла
            auto last_cursor = cursor;
            cursor = document["cursor"].GetString();
            cursor = curl.url_encode(cursor);

            if (cursor == last_cursor) {
                break;
            }
            
            GetParsedJson(app_id, cursor,  document, curl);
            if (!isSuccess(document, app_id) && isEmpty(document, app_id)) { return; }
            

        }
    }
        


    //Если общее количество ревью не равно друг другу
    if (!isTotalReviewsEqual(Json_total_reviews, coll, app_id)) {

        cursor = "*";

        

        while (true)
        {
            GetParsedJson(app_id, cursor,  document, curl);
            if (!isSuccess(document, app_id) && isEmpty(document, app_id)) { return; }
            const rapidjson::Value& reviews = document["reviews"];

            
            
            for (rapidjson::SizeType i = 0; i < reviews.Size(); i++)
            {
                    //printf( reviews[i]["timestamp_created"].GetInt() << " > " << TimeDB << std::endl;
                    //вставляем этот отзыв
                    InsertReview(reviews, coll, i, app_id);
                    DupticatesInserted++;
                
            }

            //Контроллер курсора == выхода из цикла
            auto last_cursor = cursor;
            cursor = document["cursor"].GetString();
            cursor = curl.url_encode(cursor);

            if (cursor == last_cursor) {
                break;
            }
            

        }

        if (!isTotalReviewsEqual(Json_total_reviews, coll, app_id)) {
            log.InsertCount("Recheck after inserting failed controll sum with appid:", app_id);
        }


        
    }
 

   

    //check total from reviews to current base reviews aggregation
    //if not create vector and found missing
    


    //make 2 vectors and compare for review?
    
}

void Lurker::InsertReviewByRecID(mongocxx::collection& coll, int& app_id, int64_t recID) {
    
    std::string appid_string = std::to_string(app_id);
    std::string cursor = "*";
    std::string SteamURL_sting = "https://store.steampowered.com/appreviews/" + appid_string + "?json=1" + "&filter=recent&language=all&num_per_page=100&cursor=" + cursor;
    
    CURLing curl;
    curl.GetCurlFromJson(SteamURL_sting.c_str());

    RequestCount++;
    //log.InsertCount("Request count:", RequestCount);

    //rapidjson setup
    rapidjson::Document document;
    document.Parse(curl.GetBuffer().c_str());

    cursor = document["cursor"].GetString();
    cursor = curl.url_encode(cursor);

    const rapidjson::Value& success = document["success"];

    if (success.GetInt() != 1) {
        log.InsertCount("Success false", app_id);
        return;
    }

    const rapidjson::Value& reviews = document["reviews"];

    if (reviews.Empty()) {
        //log.InsertCount("No reviews", app_id);
        return;
    }

    while (true)
    {
        //Берем текущий JSON
        for (rapidjson::SizeType i = 0; i < reviews.Size(); i++)
        {

            std::string recidStr = reviews[i]["recommendationid"].GetString();
              int recidInt = std::stoll(recidStr);
            //Если искомый recID равен рекID итератора
            if (recidInt == recID)
            {
                //вставляем этот отзыв
                InsertReview(reviews, coll, i, app_id);
                InsertedRecs++;
            }
            else {
                //если нет, берем следующий
                continue;
            }
        }


        //Контроллер курсора == выхода из цикла
        auto last_cursor = cursor;
        cursor = document["cursor"].GetString();
        cursor = curl.url_encode(cursor);

        if (cursor == last_cursor) {
            break;
        }
        SteamURL_sting = "https://store.steampowered.com/appreviews/" + appid_string + "?json=1" + "&filter=recent&language=all&num_per_page=100&cursor=" + cursor;
    }
    
}

bool Lurker::isEqualTimestamp(int64_t TimeJson, int64_t TimeDB) {

    int64_t TimeJsonIn = TimeJson;
    return TimeJson == TimeDB;
    
}

bool Lurker::isGreaterTimestamp(int64_t TimeJson, int64_t TimeDB) {
   
    return TimeJson > TimeDB;
    }

void Lurker::InsertReview(const rapidjson::Value& Doc, mongocxx::collection& coll, int i, int& app_id) {

    time_t TimeTNow = time(nullptr);

    std::string recidStr = Doc[i]["recommendationid"].GetString();
      int recidInt = std::stoll(recidStr);

    //author builder 
    const rapidjson::Value& author = Doc[i]["author"];

    std::string useridStr = author["steamid"].GetString();
      int useridInt = std::stoll(useridStr);

    auto author_insert = bsoncxx::builder::stream::document{};

    int playtime_at_review(0);
    if (author.HasMember("playtime_at_review")) {
        playtime_at_review = author["playtime_at_review"].GetInt();
    }

    author_insert

        << "steamid" << useridInt
        << "num_games_owned" << author["num_games_owned"].GetInt()
        << "num_reviews" << author["num_reviews"].GetInt()
        << "playtime_forever" << author["playtime_forever"].GetInt()
        << "playtime_last_two_weeks" << author["playtime_last_two_weeks"].GetInt()
        << "playtime_at_review" << playtime_at_review
        << "last_played" << ConverttoISODate(author["last_played"].GetInt());



    float wvcInt(0);
    if (Doc[i]["weighted_vote_score"].IsString()) {
        std::string wvcStr = Doc[i]["weighted_vote_score"].GetString();
        wvcInt = std::stof(wvcStr);
    }
    else {
        wvcInt = Doc[i]["weighted_vote_score"].GetFloat();
    }


    //main doc builder
    auto update_builder = bsoncxx::builder::stream::document{};

    update_builder

        << "recommendationid" << recidInt
        << "appid" << app_id
        << "language" << Doc[i]["language"].GetString()
        << "review" << Doc[i]["review"].GetString()
        << "timestamp_created" << ConverttoISODate(Doc[i]["timestamp_created"].GetInt())
        << "timestamp_updated" << ConverttoISODate(Doc[i]["timestamp_updated"].GetInt())
        << "voted_up" << Doc[i]["voted_up"].GetBool()
        << "votes_up" << Doc[i]["votes_up"].GetInt()
        << "votes_funny" << Doc[i]["votes_funny"].GetInt()
        << "weighted_vote_score" << wvcInt
        << "comment_count" << Doc[i]["comment_count"].GetInt()
        << "steam_purchase" << Doc[i]["steam_purchase"].GetBool()
        << "received_for_free" << Doc[i]["received_for_free"].GetBool()
        << "written_during_early_access" << Doc[i]["written_during_early_access"].GetBool()
        << "hidden_in_steam_china" << Doc[i]["hidden_in_steam_china"].GetBool()
        << "steam_china_location" << Doc[i]["steam_china_location"].GetString()
        << "author" << bsoncxx::types::b_document{ author_insert }
    << "last_update" << GetSeconds(TimeTNow);

    //coll.insert_one(update_builder.view());

    try {
        coll.insert_one(update_builder.view());
    }
    catch (const mongocxx::operation_exception& ex) {
        auto error = ex.code().value();
        if (error == 11000) {
            // Duplicate key error, continue with the loop
            //log.InsertCount("insert exception handler error", recidInt);
        }
        else {
            // Some other error occurred, handle it accordingly
            throw ex;
        }
    }

    InsertedRecs++;
}

bool Lurker::isTotalReviewsEqual(int& Json_total_reviews, mongocxx::collection& coll, int& app_id) {
    
    
    //aggregate reviews from db
    auto pipeline = mongocxx::pipeline{};
    pipeline.append_stage(
        make_document(kvp("$match", make_document(
            kvp("appid", app_id)
        ))));

    pipeline.append_stage(
        make_document(
            kvp("$group", make_document(
                kvp("_id", "$appid"),
                kvp("n_records", make_document(
                    kvp("$sum", 1)
                ))))));

    auto ReviewsAggregation = coll.aggregate(pipeline);

    int db_total_reviews = 0;
    for (auto&& Game : ReviewsAggregation) {
        //printf( bsoncxx::to_json(Game) << std::endl;
        db_total_reviews = Game["n_records"].get_int32();
        //printf( "db total reviews: " << db_total_reviews << std::endl;
    }

    //get total from json
    
    //int Json_total_reviews = Doc["query_summary"]["total_reviews"].GetInt();
    //std::cout<< "json total reviews: " << Json_total_reviews << std::endl;



    if (db_total_reviews == Json_total_reviews) {
        return true;
    }
    else {
        log.InsertCount("db_total: ", db_total_reviews);
        log.InsertCount("json_total: ", Json_total_reviews);
        log.InsertCount("diff: ", db_total_reviews - Json_total_reviews);

        log.InsertCount("DB and Json total reviews are differiet", app_id);
        return false;
    }
}


//Fullfill json_total vector with reccomendationID
void Lurker::SetJson_total(const rapidjson::Value& Doc, std::vector <int64_t>& json_total) {

    for (rapidjson::SizeType i = 0; i < Doc.Size(); i++) 
    {
        std::string recidStr = Doc[i]["recommendationid"].GetString();
          int recidInt = std::stoll(recidStr);
        json_total.push_back(recidInt);
    }
 
}

void Lurker::SetDB_total(mongocxx::collection& coll, int& app_id, std::vector <int64_t>& db_total) {
    //aggregate reviews from db
    auto pipeline = mongocxx::pipeline{};
    pipeline.append_stage(
        make_document(
            kvp("$project", make_document(
                kvp("appid", 1), kvp("recommendationid", 1)
            ))));

    pipeline.append_stage(
        make_document(kvp("$match", make_document(
            kvp("appid", app_id)
        ))));

    auto ReviewsAggregation = coll.aggregate(pipeline);

    for (auto&& Game : ReviewsAggregation) {
        db_total.push_back(Game["recommendationid"].get_int64());
    }
}

void Lurker::GetParsedJson(int& app_id, std::string cursor, rapidjson::Document& Doc, CURLing& curl ) {
    
    std::string appid_string = std::to_string(app_id);
    std::string SteamURL_sting = "https://store.steampowered.com/appreviews/" + appid_string + "?json=1" + "&filter=recent&language=all&num_per_page=100&cursor=" + cursor;
        
    curl.GetCurlFromJson(SteamURL_sting.c_str());
    RequestCount++;

    //rapidjson setup
    Doc.Parse(curl.GetBuffer().c_str());

}

bool Lurker::isSuccess(rapidjson::Document& Doc , int& app_id) {
    const rapidjson::Value& success = Doc["success"];

    if (success.GetInt() == 1) {
        return true;
    }
    else {
        //log.InsertCount("Success false appid:", app_id);
        JsonNotSuccess++;
        return false;
    }
}

bool Lurker::isEmpty(rapidjson::Document& Doc, int& app_id) {
    const rapidjson::Value& reviews = Doc["reviews"];

    if (reviews.Empty()) {
        //log.InsertCount("No reviews appid:", app_id);
        JsonNoReviews++;
        return true;
    }
    else {
        return false;
    }
}


void Lurker::goCached() {
    cached++;
    if (cached % 100 == 0) {
        log.aps(cached);
        log.InsertCount("Cached games:", cached);
    }
}