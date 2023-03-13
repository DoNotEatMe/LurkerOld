#pragma once
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "Logger.h"


class MongoDB
{
public:
    MongoDB();
    MongoDB(std::string uri);
    ~MongoDB();

    void Connect();
    void Connect(std::string uri);

    mongocxx::client* GetClient();
    mongocxx::client_session GetSession();
  

    std::string GetDatabaseName();
    std::string GetCollectionName();

    void SetDatabaseName(std::string name);
    void SetCollectionName(std::string name);

    Logger log;


private:
    

    mongocxx::instance instance;
    mongocxx::client client;

    std::string DatabaseName = "test";
    std::string CollectionName = "default";
    
    

    
   

};
