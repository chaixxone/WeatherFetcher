#ifndef WEATHER_MAPPER_INTERFACE_HPP
#define WEATHER_MAPPER_INTERFACE_HPP

#include <string>

class IWeatherMapper
{
public:
	virtual void FetchWeatherData(const std::string& city) = 0;
};

#endif