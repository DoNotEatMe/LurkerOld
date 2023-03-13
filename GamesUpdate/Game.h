#pragma once
#include <string>
#include <vector>


#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>





class Game
{
public:
	Game(int AppId, std::string Title);
	
	int AppId;
	std::string Title;
	
	bsoncxx::document::value doc();

	~Game() {};
};

