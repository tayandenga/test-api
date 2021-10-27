#include "server.hpp"
#include "ini.hpp"

int main(int argc, char* argv[])
{
	try
	{
		mINI::INIStructure config;
		if(!mINI::INIFile("config.ini").read(config))
		{
			std::cerr << "Malformed config.ini file, exiting" << std::endl;
			return 1;
		}

		if(!config.has("database"))
		{
			std::cerr << "Missing database connection data in config.ini, exiting" << std::endl;
			return 1;
		}

		int port = 3306;
		if(config["database"].has("port"))
			port = atoi(config["database"].get("port").c_str());

		Database::DriverPtr db = std::make_shared<Database::Driver>(
			config["database"].get("host"),
			config["database"].get("user"),
			config["database"].get("password"),
			config["database"].get("database"),
			port
		);
#ifndef DEBUG
		if(!db->isConnected())
		{
			std::cerr << "Could not connect to main database, exiting" << std::endl;
			return 1;
		}
#endif

		Database::DriverPtr logger = nullptr;
		if(config.has("logger"))
		{
			port = 3306;
			if(config["logger"].has("port"))
				port = atoi(config["logger"].get("port").c_str());

			logger = std::make_shared<Database::Driver>(
				config["logger"].get("host"),
				config["logger"].get("user"),
				config["logger"].get("password"),
				config["logger"].get("logger"),
				port
			);
#ifndef DEBUG
			if(!logger->isConnected())
				std::cout << "Could not connect to logger database, continue without logging!" << std::endl;
#endif
		}

		port = 34543;
		if(config.has("api") && config["api"].has("port"))
			port = atoi(config["api"].get("port").c_str());

		boost::asio::io_context io;
		ServerPtr s = std::make_shared<Server>(io, port, std::make_shared<Api>(db, logger));
		io.run();
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
