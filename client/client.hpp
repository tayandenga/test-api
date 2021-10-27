#include <iostream>
#include <boost/asio.hpp>

#define MAX_PACKET_SIZE 16384

class Client
{
	private:
                boost::asio::ip::tcp::socket socket;
		boost::asio::ip::tcp::resolver::results_type endpoints;

                std::array<char, MAX_PACKET_SIZE> buffer;
		void connect()
		{
			boost::asio::async_connect(socket, endpoints, [this](const boost::system::error_code& err, boost::asio::ip::tcp::endpoint)
			{
				if(!err)
				{
#ifdef DEBUG
					std::cout << "[DEBUG] Connected" << std::endl;
#endif
					read();
				}
				else
				{
					std::cerr << "Connect error: " << err.message() << std::endl;
					socket.close();
				}
			});
		}

		void read()
		{
#ifdef DEBUG
			std::cout << "[DEBUG] Awaiting read" << std::endl;
#endif
			socket.async_read_some(boost::asio::buffer(buffer, MAX_PACKET_SIZE), [=](const boost::system::error_code& err, size_t bytes_transferred)
			{
				if(!err)
				{
#ifdef DEBUG
					std::cout << "[DEBUG] Received data:" << std::endl;
#endif
					std::cout.write(buffer.data(), bytes_transferred);
					read();
				}
				else if(socket.is_open())
				{
					socket.close();
					if(err != boost::asio::error::eof)
						std::cerr << "Read error: " << err.message() << std::endl;
				}
			});
		}

		void write(const std::string& msg)
		{
#ifdef DEBUG
			std::cout << "[DEBUG] Awaiting write" << std::endl;
#endif
			socket.async_write_some(boost::asio::buffer(msg, MAX_PACKET_SIZE), [=](const boost::system::error_code& err, size_t bytes_transferred)
			{
				if(err)
				{
					std::cerr << "Write error: " << err.message() << std::endl;
					socket.close();
				}
#ifdef DEBUG
				else
					std::cout << "[DEBUG] Sent data" << std::endl;
#endif
			});
		}

	public:
		Client(boost::asio::io_context& io_, const std::string& host, const std::string& port) :
			socket(io_)
		{
			boost::asio::ip::tcp::resolver resolver(io_);
			endpoints = resolver.resolve(host.c_str(), port.c_str());
			connect();
		}

		void disconnect()
		{
#ifdef DEBUG
			std::cout << "[DEBUG] Disconnecting" << std::endl;
#endif
			boost::asio::post(socket.get_io_context(), [=]()
			{
				socket.close();
			});
		}

		void post(const std::string& msg)
		{
			boost::asio::post(socket.get_io_context(), [=]()
			{
				write(msg);
			});
		}

		bool isConnected() const {return socket.is_open();}
};
typedef std::shared_ptr<Client> ClientPtr;
