find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})
add_executable(server "main.cpp" "api.cpp" "api.hpp" "database.hpp" "database.cpp" "session.cpp" "session.hpp" "server.hpp" "ini.hpp")
target_link_libraries (server ${MYSQL_LIBRARIES} Boost::regex Threads::Threads JsonCpp::JsonCpp)
