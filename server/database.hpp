#include <sstream>
#include <map>
#include <memory>
#include <mysql/mysql.h>

namespace Database
{
	typedef std::ostringstream Query;

	class Result
	{
		private:
			MYSQL_RES* m_handle;
			MYSQL_ROW m_row;

			typedef std::map<std::string, unsigned int> ListNames_t;
			ListNames_t m_listNames;

		public:
			Result(MYSQL_RES*);
			~Result();
			bool next();

			int getInt(const std::string&) const;
			long long getLong(const std::string&) const;
			std::string getString(const std::string&) const;
	};
	typedef std::shared_ptr<Result> ResultPtr;

	class Driver
	{
		private:
			MYSQL m_handle;
			bool m_connected;

		public:
			Driver(const std::string&, const std::string&, const std::string&, const std::string&, int);
			~Driver();
			std::string escape(const std::string&);

			bool execute(const std::string&);
			bool execute(const Query& q) {return execute(q.str());}
			ResultPtr store(const std::string&);
			ResultPtr store(const Query& q) {return store(q.str());}

			bool isConnected() const {return m_connected;}
			unsigned long long getLastInsertId() {return (unsigned long long)(m_connected ? mysql_insert_id(&m_handle) : -1);}
	};
	typedef std::shared_ptr<Driver> DriverPtr;
};
