#ifndef WEATHER_MAPPER_INTERFACE_HPP
#define WEATHER_MAPPER_INTERFACE_HPP

#include <string>
#include "weather_data.hpp"

class IWeatherMapper
{
public:
	virtual WeatherData FetchWeatherData(const std::string& city) = 0;
};

#endif