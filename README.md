# Idea and WIP
	Create auto renewing DB with Steam games info for upper analytics and researches. 
	Add all reviews for each game to review
	Create web interface to interact with

# Lurker

This code was made in educational purposes and not pretended to hurt any company/person. Please contact me directly (t.me/donoteatme) if you think this code contains any sesitive information
 
Uncompleted.
Complex of server programm solutions to parcing steam data and store into MongoDB
 
/GamesUpdate/
	- Storing all games and updating main info direct from steam

/Proxyholder/
	- Scrapping and storing IP's for proxy connections




Lurker.cpp
	- Initial point for main functions

Lurker::AllGamesInsertMany()
	- insert_many of DB is empty. Best way to get initial parcing info.
Lurker::UpdateGames()
	- Comparison of two arrays of appID's (DB + Steam actual) and adding new to DB.
Lurker::AddGameInfo()
	- If game has no last_update marker, adding game info by getting direct info from steam. Also assign last_update datetime for new appIDs

Logger
	- Complex log system

CURLing.cpp
	- cURL setup reusable class with specific requests

MongoDB.cpp
	- MongoDB setup reusable class




