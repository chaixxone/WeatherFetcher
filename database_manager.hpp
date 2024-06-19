#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <stdexcept>
#include <fstream>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include "database_manager_interface.hpp"
#include "weather_data.hpp"

class DB_Manager : IDB_Manager
{
public:
	DB_Manager();
	void Write(std::unique_ptr<WeatherData> weatherData) override;
	WeatherData ReadLast() override;
	std::string GetKey() override;

private:
	std::string _getPassword(const std::string& pathToFile);
	const std::string m_server;
	const std::string m_username;
	const std::string m_password;
	sql::Driver* m_driver;
};

#endif