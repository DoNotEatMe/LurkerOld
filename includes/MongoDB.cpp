#include "MongoDB.h"
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/exception/logic_error.hpp>

MongoDB::MongoDB()
{
    log.Connection("Connection created");
    Connect();
}

MongoDB::MongoDB(std::string uri)
{
    Connect(uri);
}

MongoDB::~MongoDB() {
    
    log.Connection("Connection closed");
}


//Create connection to localhost without password
void MongoDB::Connect()
{

    try {
        client = mongocxx::client{ mongocxx::uri{"mongodb://Pin:3mZzjmeWy5Q5h2IV6W7uYfZ@81.200.146.75:27017/?authSource=admin"} };
    }
    catch (const mongocxx::exception& e) {
        std::cerr << "Error creating connection: " << e.what() << std::endl;
        log.Connection("Error creating connection: ",e.what());
    }

    //bad Assertion. In case of right log/pwd connection is secceed.
    if (client) {
        log.Connection("localhost connection succeed");
    }
    else {
        log.Connection("Failed to create mongocxx::client");
        
    }
  
}


//Create connection to URL without password
void MongoDB::Connect(std::string uri)
{

    try {
    client = mongocxx::client{ mongocxx::uri{uri} };
    }
    catch (const mongocxx::exception& e) {
        std::cerr << "Error creating connection: " << e.what() << std::endl;
        log.Connection("Error creating connection: ", e.what());
    }

    if (client) {
        log.Connection(uri, "connection succeed");
    }
    else {
        log.Connection("Failed to create mongocxx::client", uri);
    }

  

}


//returns client instance to operate with DB
mongocxx::client* MongoDB::GetClient() {
    if (client){
    return &client;
    }
    else {
        log.Connection("MongoDB::GetClient() Error: client not exist");
    }
    return NULL;
}

mongocxx::client_session MongoDB::GetSession() {
    return client.start_session();
}



std::string MongoDB::GetDatabaseName() {
    return this->DatabaseName;
}
std::string MongoDB::GetCollectionName() {
    return this->CollectionName;
}



void MongoDB::SetDatabaseName(std::string name) {
    this->DatabaseName = name;
}

void MongoDB::SetCollectionName(std::string name) {
    this->CollectionName = name;
}



