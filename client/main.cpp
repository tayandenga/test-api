#include "client.hpp"

int main(int argc, char* argv[])
{
	try
	{
		if(argc < 3)
		{
			std::cerr << "Missing <host> and <port> arguments, exiting\n";
			return 1;
		}

		boost::asio::io_context io;
		ClientPtr client = std::make_shared<Client>(io, argv[1], argv[2]);
		std::thread console([client]()
		{
			auto reader = []() {
				std::string i;
				std::getline(std::cin, i);
				return i;
			};

			std::future<std::string> future = std::async(reader);
			while(client->isConnected())
			{
				if(future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
				{
					std::string cmd = future.get();
					client->post(cmd);
					future = std::async(reader);
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		});

		io.run();
		console.detach();
		client->disconnect();
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
