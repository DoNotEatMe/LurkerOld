#include "Game.h"



Game::Game(int AppId, std::string Title)
{
	this->AppId = AppId;
	
	this->Title = Title;
	
	//std::string URLCompile = "https://store.steampowered.com/app/" + AppId;
	//this->URL = "URLCompile";
}


bsoncxx::document::value Game::doc()
{
	auto builder = bsoncxx::builder::stream::document{};
	bsoncxx::document::value doc_value = builder
		<< "appid" << this->AppId
		<< "name" << this->Title
		<< bsoncxx::builder::stream::finalize;
	return doc_value;
}




