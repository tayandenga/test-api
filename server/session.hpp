#include <iostream>
#include <functional>
#include <thread>
#include <boost/asio.hpp>

#define MAX_PACKET_SIZE 16384

class Server;
typedef std::shared_ptr<Server> ServerPtr;

class Api;
typedef std::shared_ptr<Api> ApiPtr;

class Session : public std::enable_shared_from_this<Session>
{
	private:
		std::chrono::time_point<std::chrono::system_clock> start;
		boost::asio::ip::tcp::socket socket;
		boost::asio::deadline_timer timeout;
		ServerPtr server;

		std::array<char, MAX_PACKET_SIZE> buffer;
		void run();

	protected:
		friend class Server;
                void onDisconnect();

	public:
		Session(boost::asio::ip::tcp::socket socket_, ServerPtr server_) : start(std::chrono::system_clock::now()), socket(std::move(socket_)), timeout(std::move(socket_).get_io_context()), server(server_) {run();}
		int getUptime() const {return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();}

		ServerPtr getServer() const {return server;}
		boost::asio::io_context& getIO() {return socket.get_io_context();}
		ApiPtr getHandler() const;

		std::string getIP() const
		{
			boost::system::error_code err;
			const boost::asio::ip::tcp::endpoint endpoint = socket.remote_endpoint(err);
			if(err)
				return std::string();

			return endpoint.address().to_v4().to_string();
		}
};
typedef std::shared_ptr<Session> SessionPtr;
