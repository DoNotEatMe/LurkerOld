# CMakeLists.txt at the root of Lurker project

cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

project(Lurker)

# Set the path to vcpkg
	set(VCPKG_ROOT "/root/vcpkg")

# Set the triplet
	set(VCPKG_TARGET_TRIPLET "x64-linux")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories for GameUpdate and ProxyHolder projects
add_subdirectory(GamesUpdate)
add_subdirectory(ProxyHolder)
add_subdirectory(Tests)

# Add include directories for self-made classes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

include_directories(${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include)

set(CURL_DIR /root/vcpkg/installed/x64-linux/share/curl/)
set(bsoncxx_DIR /root/vcpkg/installed/x64-linux/share/bsoncxx/)
set(bson-1.0_DIR /root/vcpkg/installed/x64-linux/share/bson-1.0/)
set(mongocxx_DIR /root/vcpkg/installed/x64-linux/share/mongocxx/)
set(mongoc-1.0_DIR /root/vcpkg/installed/x64-linux/share/mongoc-1.0/)
set(RapidJSON_DIR /root/vcpkg/installed/x64-linux/share/rapidjson/)

# Add vcpkg dependencies
find_package(CURL CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

add_library(LurkerIncludes INTERFACE)
target_include_directories(LurkerIncludes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/includes)


#	Games Update
	add_executable(GamesUpdate GamesUpdate/GamesUpdate.cpp
								includes/Logger.cpp
								includes/Game.cpp
								includes/CURLing.cpp
								includes/MongoDB.cpp
								includes/Lurker.cpp
								includes/IP.cpp
								)

	install(TARGETS GamesUpdate
			RUNTIME DESTINATION /root/projects/Lurker/GamesUpdate/)

	target_link_libraries(GamesUpdate PRIVATE CURL::libcurl
									$<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
									$<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
									rapidjson)


#	ProxyHolder
	add_executable(ProxyHolder ProxyHolder/ProxyHolder.cpp
								includes/CURLing.cpp
								includes/MongoDB.cpp
								includes/Logger.cpp
								includes/IP.cpp
								)

	install(TARGETS ProxyHolder
			RUNTIME DESTINATION /root/projects/Lurker/ProxyHolder/)

	target_link_libraries(ProxyHolder PRIVATE CURL::libcurl
									$<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
									$<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
									rapidjson)


#	tests
	add_executable(Tests Tests/Tests.cpp
							includes/Logger.cpp
							includes/Game.cpp
							includes/CURLing.cpp
							includes/MongoDB.cpp
							includes/Lurker.cpp
							includes/IP.cpp)

	install(TARGETS Tests
			RUNTIME DESTINATION /root/projects/Lurker/Tests/)

	target_link_libraries(Tests PRIVATE CURL::libcurl
									$<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
									$<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
									rapidjson)

