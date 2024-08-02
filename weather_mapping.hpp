#ifndef WEATHER_MAPPING_HPP
#define WEATHER_MAPPING_HPP

#pragma once
#include <string>
#include <ctime>
#include <cpr/cpr.h>
#include "json.hpp"
#include "weather_mapper_interface.hpp"
#include "database_manager.hpp"

using json = nlohmann::json;

class WeatherMapper : IWeatherMapper
{
public:
	WeatherMapper(const std::unordered_map<std::string, std::string>& config);
	std::string FetchWeatherData(const std::string& city) override;

private:
	std::string m_SECRET_API_KEY;
};

#endif