cmake_minimum_required (VERSION 3.8)
project (GamesUpdater VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the path to vcpkg
	set(VCPKG_ROOT "/root/vcpkg")

# Set the triplet
	set(VCPKG_TARGET_TRIPLET "x64-linux")

	set(CMAKE_INSTALL_PREFIX "/root/projects/Lurker/UpdateGames/" CACHE PATH "Installation directory")
	
	#install(FILES file DESTINATION /root/projects/Lurker/UpdateGames/lib)

	add_compile_definitions(PROJECT_NAME="GamesUpdate")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")




set(CURL_DIR /root/vcpkg/installed/x64-linux/share/curl/)
set(bsoncxx_DIR /root/vcpkg/installed/x64-linux/share/bsoncxx/)
set(bson-1.0_DIR /root/vcpkg/installed/x64-linux/share/bson-1.0/)
set(mongocxx_DIR /root/vcpkg/installed/x64-linux/share/mongocxx/)
set(mongoc-1.0_DIR /root/vcpkg/installed/x64-linux/share/mongoc-1.0/)
set(RapidJSON_DIR /root/vcpkg/installed/x64-linux/share/rapidjson/)

# Include directories for vcpkg dependencies
	include_directories(${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include)

# Link directories for vcpkg dependencies
	#link_directories(${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib)
	

# Find vcpkg packages
	find_package(CURL CONFIG REQUIRED)
	find_package(bsoncxx CONFIG REQUIRED)
	find_package(mongocxx CONFIG REQUIRED)
	find_package(RapidJSON CONFIG REQUIRED)



# Add the executable target
	add_executable(GamesUpdate	"GamesUpdate.cpp"
								"Logger.cpp"
								"Game.cpp"
								"CURLing.cpp"
								"MongoDB.cpp"
								"Lurker.cpp")	


# My cpp classes
#add_library(Logger STATIC ${PROJECT_SOURCE_DIR}/Logger.cpp)
#add_library(Game STATIC ${PROJECT_SOURCE_DIR}/Game.cpp)
#add_library(CURLing STATIC ${PROJECT_SOURCE_DIR}/CURLing.cpp)
#add_library(MongoDB STATIC ${PROJECT_SOURCE_DIR}/MongoDB.cpp)
#add_library(Lurker STATIC ${PROJECT_SOURCE_DIR}/Lurker.cpp)

# Link custom libraries
	#target_link_libraries(GamesUpdate PUBLIC Lurker  Game CURLing)



	install(TARGETS GamesUpdate DESTINATION /root/projects/Lurker/GamesUpdate)



# Link libraries
	target_link_libraries(GamesUpdate PRIVATE CURL::libcurl)
	target_link_libraries(GamesUpdate PRIVATE $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>)
	target_link_libraries(GamesUpdate PRIVATE $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>)
	target_link_libraries(GamesUpdate PRIVATE rapidjson)
	
