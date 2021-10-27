#include <map>
#include <json/json.h>

#include "session.hpp"
#include "database.hpp"

class Api
{
	private:
		int queryCount;

		Database::DriverPtr db;
		Database::DriverPtr log;

	public:
		Api(const Database::DriverPtr& db_, const Database::DriverPtr& log_) : db(db_), log(log_), queryCount(0) {}
		int getQueryCount() const {return queryCount;}

		void Command(SessionPtr, std::string, std::function<void(const std::string&, int, bool)>);
		void Log(const std::string&, const std::string&);
};
//typedef std::shared_ptr<Api> ApiPtr;
