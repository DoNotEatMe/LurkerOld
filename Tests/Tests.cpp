#include "CURLing.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "MongoDB.h"

int main() {

	CURLing curl;
    MongoDB db;

    db.SetDatabaseName("steam");
    db.SetCollectionName("proxylist");
    auto coll = db.GetClient()->database(db.GetDatabaseName()).collection(db.GetCollectionName());


	int count(0);

    std::cout << "Getting appid..." << std::endl;
    std::ifstream input_file("appid.csv");
    if (!input_file.is_open()) {
        std::cerr << "Failed to open input file" << std::endl;
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input_file, line)) {
        if (line.length() > 0) {
            lines.push_back(line);
        }
    }

    // Close the input file
    input_file.close();
    std::cout << "Succeed!" << std::endl;



	for (auto& appid : lines){

        std::string appid_clean = appid;
        appid_clean.erase(0, appid_clean.find_first_not_of(" \t\n\r\f\v"));
        appid_clean.erase(appid_clean.find_last_not_of(" \t\n\r\f\v") + 1);

		curl.ProxyTest(appid_clean, coll);
		count++;
		std::cout << "count " << count << std::endl;
    }
	
	

	return 0;
}