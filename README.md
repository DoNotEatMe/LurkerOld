# Steam Game Database

## Idea and Work in Progress
This project aims to create an auto-renewing database that stores information about Steam games for upper analytics and research purposes. It includes the following features:

- Collects and stores all reviews for each game
- Provides a web interface for interaction

## Disclaimer
This code was created for educational purposes and is not intended to harm any company or individual. If you believe this code contains any sensitive information, please contact me directly at [t.me/donoteatme](https://t.me/donoteatme).

## Project Structure

### GamesUpdate
- This component is responsible for storing all games and updating their main information directly from Steam.

### Proxyholder
- This component is responsible for scraping and storing IP addresses for proxy connections.

### Lurker.cpp
- This is the initial entry point for the main functions of the project.

### Lurker::AllGamesInsertMany()
- Inserts information about all games when the database is empty. This is the best way to get initial parsing information.

### Lurker::UpdateGames()
- Compares two arrays of appIDs (one from the database and the other from Steam) and adds new games to the database.

### Lurker::AddGameInfo()
- Adds game information for appIDs that have no "last_update" marker by directly fetching information from Steam. It also assigns the "last_update" datetime for new appIDs.

### Logger
- This component provides a complex log system.

### CURLing.cpp
- This class sets up reusable cURL requests.

### MongoDB.cpp
- This class sets up the connection to MongoDB and provides reusability.

