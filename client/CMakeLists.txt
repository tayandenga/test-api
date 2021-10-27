add_executable (client "main.cpp" "client.hpp")
target_link_libraries (client ${BOOST_LIBRARIES} Threads::Threads)
