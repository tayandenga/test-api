#include "server.hpp"

#define API_CMD(name) {#name, [](Database::DriverPtr db, SessionPtr session, int& delay, Json::Value& in, Json::Value& out)
#define API_LOG(txt) session->getHandler()->Log(txt, session->getIP())

const std::map<std::string, std::function<void(Database::DriverPtr, SessionPtr, int&, Json::Value&, Json::Value&)>> Commands = {
	API_CMD(END) {
		//
	}},
	API_CMD(STAT) {
		Json::Value root;
		root["length"] = session->getUptime();

		root["uptime"] = session->getServer()->getUptime();
		root["sessions"] = session->getServer()->getSessionCount();

		root["queries"] = session->getHandler()->getQueryCount();
		out["args"] = root;
	}},
	API_CMD(INC) {
		std::string numStr = in["number"].asString();
		long long number = in["number"].asInt64();
		Database::Query query;

		query << "SELECT `count` FROM `inc` WHERE `key` = " << number;
		Database::ResultPtr result = db->store(query);
		API_LOG(numStr + " hit");

		query.str("");
		if(result != nullptr)
		{
			query << "UPDATE `inc` SET `count` = `count` + 1 WHERE `key` = " << number;
			db->execute(query);
			out["hits"] = result->getLong("count");
		}
		else
		{
			query << "INSERT INTO `inc` (`key`, `count`) VALUES (" << number << ", 1)";
			db->execute(query);
			out["hits"] = 0;
		}
	}},
	API_CMD(GET) {
		Database::Query query;
		query << "SELECT `count` FROM `inc` WHERE `key` = " << in["number"].asInt64();

		Database::ResultPtr result = db->store(query);
		if(result != nullptr)
			out["hits"] = result->getLong("count");
		else
			out["hits"] = 0;
	}},
	API_CMD(SLEEP) {
		delay = (int)in["delay"].asUInt();
	}},
	API_CMD(WRITE) {
		std::string key = in["key"].asString();
		std::string value = in["value"].asString();
		Database::Query query;

		query << "SELECT `value` FROM `pairs` WHERE `key` = " << db->escape(key);
		Database::ResultPtr result = db->store(query);

		query.str("");
		if(result != nullptr)
		{
			query << "UPDATE `pairs` SET `value` = " << db->escape(value) << " WHERE `key` = " << db->escape(key);
			db->execute(query);
			API_LOG(key + " changed to " + value);
		}
		else
		{
			query << "INSERT INTO `pairs` (`key`, `value`) VALUES (" << db->escape(key) << ", " << db->escape(value) << ")";
				db->execute(query);
			API_LOG(key + " set to " + value);
		}
	}},
	API_CMD(READ) {
		Database::Query query;
		query << "SELECT `value` FROM `pairs` WHERE `key` = " << db->escape(in["key"].asString());

		Database::ResultPtr result = db->store(query);
		if(result != nullptr)
			out["value"] = result->getString("value");
		else
			out["value"] = 0;
	}},

	API_CMD(DEL) {
		std::string key = in["key"].asString();
		Database::Query query;

		query << "DELETE FROM `pairs` WHERE `key` = " << db->escape(key);
		db->execute(query);
		API_LOG(key + " deleted");
	}}
};

void Api::Command(SessionPtr session, std::string data, std::function<void(const std::string&, int, bool)> callback)
{
	Json::Value input, output;
	bool finish = false;
	int delay = -1;

	++queryCount;
	try
	{
		Json::Reader reader;
		reader.parse(data, input);

		std::string cmd = input["cmd"].asString();
		if(cmd.length() > 0 && Commands.count(cmd) == 1)
		{
			output["status"] = "ok";
			Commands.at(cmd)(db, session, delay, input["args"], output);
			if(cmd == "END")
				finish = true;
		}
		else
			output["status"] = "error";
	}
	catch(std::exception& e)
	{
		output["status"] = "error";
	}

	Json::FastWriter buffer;
	callback((std::string)buffer.write(output), delay, finish);
}

void Api::Log(const std::string& text, const std::string& ip)
{
	if(log == nullptr)
		return;

	Database::Query query;
	query << "INSERT INTO `log` (`info`, `ip`, `timestamp`) VALUES (" << log->escape(text) << ", " << log->escape(ip) << ", UNIX_TIMESTAMP())";
	log->execute(query);
}

