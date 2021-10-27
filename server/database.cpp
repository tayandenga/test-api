#include "database.hpp"
#include <iostream>
#include <mysql/errmsg.h>
#include <boost/regex.hpp>

using namespace Database;

Driver::Driver(const std::string& mysqlHost, const std::string& mysqlUser, const std::string& mysqlPassword, const std::string& mysqlDatabase, int mysqlPort)
{
	m_connected = false;
	if(mysql_init(&m_handle))
	{
		my_bool reconnect = true;
		mysql_options(&m_handle, MYSQL_OPT_RECONNECT, &reconnect);
		if(mysql_real_connect(&m_handle, mysqlHost.c_str(), mysqlUser.c_str(), mysqlPassword.c_str(), mysqlDatabase.c_str(), mysqlPort, NULL, 0))
			m_connected = true;
		else
			std::cerr << "Failed to connect to database: " << mysql_error(&m_handle) << std::endl;
	}
	else
		std::cerr << "Failed to initialize MySQL connection handle" << std::endl;
}

Driver::~Driver()
{
	mysql_close(&m_handle);
}

std::string Driver::escape(const std::string& s)
{
	char* output = new char[s.length() * 2 + 1];
	mysql_real_escape_string(&m_handle, output, s.c_str(), s.length());

	std::string r = "'";
	r += output;
	r += "'";

	delete[] output;
	return r;
}

bool Driver::execute(const std::string& query)
{
	if(!m_connected)
		return false;

	bool state = true;
	if(mysql_real_query(&m_handle, query.c_str(), query.length()) != 0)
	{
		int error = mysql_errno(&m_handle);
		if(error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
			m_connected = false;

		state = false;
	}

	MYSQL_RES* result = mysql_store_result(&m_handle);
	if(result)
		mysql_free_result(result);

	return state;
}

ResultPtr Driver::store(const std::string& query)
{
	if(!m_connected)
		return nullptr;

	if(mysql_real_query(&m_handle, query.c_str(), query.length()) != 0)
	{
		int error = mysql_errno(&m_handle);
		if(error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
			m_connected = false;
	}

	MYSQL_RES* result = mysql_store_result(&m_handle);
	if(!result)
	{
		int error = mysql_errno(&m_handle);
		if(error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
			m_connected = false;

		return nullptr;
	}

	ResultPtr r = std::make_shared<Result>(result);
	if(r == nullptr || !r->next())
		return nullptr;

	return r;
}

Result::Result(MYSQL_RES* res)
{
	m_handle = res;
	m_listNames.clear();

	MYSQL_FIELD* field;
	int i = 0;
	while((field = mysql_fetch_field(m_handle)))
	{
		m_listNames[field->name] = i;
		i++;
	}
}

Result::~Result()
{
	mysql_free_result(m_handle);
}

bool Result::next()
{
	m_row = mysql_fetch_row(m_handle);
	return m_row != NULL;
}

int Result::getInt(const std::string& s) const
{
	ListNames_t::const_iterator it = m_listNames.find(s);
	if(it == m_listNames.end())
		return 0;

	if(m_row[it->second] == NULL)
		return 0;

	return atoi(m_row[it->second]);
}

long long Result::getLong(const std::string& s) const
{
	ListNames_t::const_iterator it = m_listNames.find(s);
	if(it == m_listNames.end())
		return 0;

	if(m_row[it->second] == NULL)
		return 0;

	return atoll(m_row[it->second]);
}

std::string Result::getString(const std::string& s) const
{
	ListNames_t::const_iterator it = m_listNames.find(s);
	if(it == m_listNames.end())
		return std::string();

	if(m_row[it->second] == NULL)
		return std::string();

	return std::string(m_row[it->second]);
}
