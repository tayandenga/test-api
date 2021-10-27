#include "server.hpp"

void Session::onDisconnect()
{
	timeout.cancel();
	socket.close();
}

void Session::run()
{
	timeout.expires_from_now(boost::posix_time::seconds(30));
	timeout.async_wait([this](const boost::system::error_code& err)
	{
		if(!err)
		{
#ifdef DEBUG
			std::cout << "[DEBUG] Session timed out" << std::endl;
#endif
			server->disconnect(shared_from_this());
		}
	});

#ifdef DEBUG
	std::cout << "[DEBUG] Await read" << std::endl;
#endif
	socket.async_read_some(boost::asio::buffer(buffer, MAX_PACKET_SIZE), [this](const boost::system::error_code& err, size_t bytes_transferred)
	{
		timeout.cancel();
		if(!err)
		{
#ifdef DEBUG
			std::cout << "[DEBUG] Read client data" << std::endl;
#endif
			server->getHandler()->Command(shared_from_this(), std::string(buffer.data(), bytes_transferred), [this](const std::string& response, int delay, bool finish)
			{
				auto cb = [this, response, finish]() {
#ifdef DEBUG
					std::cout << "[DEBUG] Writing response: " << response << std::endl;
#endif
					socket.async_write_some(boost::asio::buffer(response, MAX_PACKET_SIZE), [this, finish](const boost::system::error_code& err, size_t bytes_transferred)
					{
						if(finish)
						{
#ifdef DEBUG
							std::cout << "[DEBUG] Write ok, disconnecting" << std::endl;
#endif
							server->disconnect(shared_from_this());
						}
						else if(!err)
						{
#ifdef DEBUG
							std::cout << "[DEBUG] Write ok" << std::endl;
#endif
							run();
						}
						else
						{
							std::cerr << "Write error: " << err.message() << std::endl;
							server->disconnect(shared_from_this());
						}
					});

				};
				if(delay != -1)
				{
					timeout.expires_from_now(boost::posix_time::seconds(delay));
					timeout.async_wait([=](const boost::system::error_code& err)
					{
						if(!err)
							cb();
					});
				}
				else
					cb();
			});
		}
		else if(socket.is_open())
		{
			server->disconnect(shared_from_this());
			if(err != boost::asio::error::eof)
				std::cerr << "Read error: " << err.message() << std::endl;
		}
	});
}

ApiPtr Session::getHandler() const
{
	return server->getHandler();
}
