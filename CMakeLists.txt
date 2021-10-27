cmake_minimum_required (VERSION 3.8)
project (wazdan C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread regex)
find_package(JsonCpp REQUIRED)

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY -DDEBUG)
include_directories(${BOOST_INCLUDE_DIRS} ${JSONCPP_INCLUDE_DIRS})

add_subdirectory("client")
add_subdirectory("server")
