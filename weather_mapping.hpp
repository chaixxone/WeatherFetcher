#ifndef WEATHER_MAPPING_HPP
#define WEATHER_MAPPING_HPP

// Just an example of API call
// https://api.openweathermap.org/data/2.5/weather?q=London&appid={API key}
// Response type is JSON data

#include <string>
#include <boost/asio.hpp>
#include "json.hpp"
#include "weather_mapper_interface.hpp"

using namespace boost::asio;

class WeatherMapper : IWeatherMapper
{
public:
	WeatherMapper();
	void FetchWeatherData(const std::string& city) override;

private:
	io_context m_io_context;
	ip::tcp::socket m_socket;
	ip::tcp::resolver m_resolver;
	const std::string m_SECRET_API_KEY;
};

#endif