// Cmaketest.cpp : Defines the entry point for the application.
//

#include "GamesUpdate.h"
#include "Lurker.h"
#include "Logger.h"


int main()
{
	Logger loger;

	

	

	Lurker Lurk;
	Lurk.SetDatabaseName("steam");
	Lurk.SetCollectionName("games");
	
	
	loger.SetExecutionTimer();
	Lurk.UpdateGames();

	if (Lurk.DocArr.size()>0){
		Lurk.AddGameInfo(Lurk.DocArr);
	}

	loger.ExecutionTime("program time");
		
	
	
	    
	return 0;
}


