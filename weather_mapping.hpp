#ifndef WEATHER_MAPPING_HPP
#define WEATHER_MAPPING_HPP

#pragma once
#include <string>
#include <ctime>
#include <boost/asio.hpp>
#include "json.hpp"
#include "weather_mapper_interface.hpp"
#include "database_manager.hpp"

using namespace boost::asio;
using json = nlohmann::json;

class WeatherMapper : IWeatherMapper
{
public:
	WeatherMapper();
	WeatherData FetchWeatherData(const std::string& city) override;

private:
	io_context m_io_context;
	ip::tcp::socket m_socket;
	ip::tcp::resolver m_resolver;
	std::string m_SECRET_API_KEY;
};

#endif