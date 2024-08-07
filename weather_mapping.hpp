#ifndef WEATHER_MAPPING_HPP
#define WEATHER_MAPPING_HPP

#pragma once
#include <string>
#include <ctime>
#include <cpr/cpr.h>
#include "json.hpp"

using json = nlohmann::json;

class WeatherMapper
{
public:
	WeatherMapper(const std::string& apiKey);
	std::string FetchWeatherData(const std::string& city) const;

private:
	std::string m_SECRET_API_KEY;
};

#endif