#include <set>
#include "api.hpp"

class Server : public std::enable_shared_from_this<Server>
{
	private:
		std::chrono::time_point<std::chrono::system_clock> start;
		boost::asio::ip::tcp::acceptor acceptor;
		ApiPtr handler;

		typedef std::set<SessionPtr> Sessions_t;
		Sessions_t sessions;

		void run()
		{
			acceptor.async_accept([this](const boost::system::error_code& err, boost::asio::ip::tcp::socket socket)
			{
				if(!err)
				{
#ifdef DEBUG
					std::cout << "[DEBUG] New session" << std::endl;
#endif
					sessions.insert(std::make_shared<Session>(std::move(socket), shared_from_this()));
				}

				run();
			});
		}

	protected:
		friend class Session;
		void disconnect(SessionPtr session)
		{
			Sessions_t::iterator it = sessions.find(session);
			if(it != sessions.end())
				sessions.erase(it);

			session->onDisconnect();
		}

	public:
		Server(boost::asio::io_context& io, int port, ApiPtr app) : acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), handler(app), start(std::chrono::system_clock::now()) {run();}
		int getUptime() const {return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();}

		ApiPtr getHandler() const {return handler;}
		int getSessionCount() const {return sessions.size();}
};
//typedef std::shared_ptr<Server> ServerPtr;
