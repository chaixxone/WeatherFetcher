#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>
#include "weather_data.hpp"

class DB_Manager
{
public:
	DB_Manager(const std::unordered_map<std::string, std::string>& config);
	void Write(std::unique_ptr<WeatherData> weatherData);
	WeatherData ReadLast(const std::string& city);
	std::string GetKey();
	void LoadImageData(const std::string& imagePath, const std::string& name);
	std::string GetImageData(const std::string& imageName);

private:
	std::string _getPassword(const std::string& pathToFile);
	const std::string m_server;
	const std::string m_username;
	const std::string m_password;
	sql::Driver* m_driver;
	std::unique_ptr<sql::Connection> m_connection;
};

#endif