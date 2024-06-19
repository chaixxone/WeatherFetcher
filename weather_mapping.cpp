#include "weather_mapping.hpp"

WeatherMapper::WeatherMapper() : m_io_context(), m_socket(m_io_context), m_resolver(m_io_context) { }

void WeatherMapper::FetchWeatherData(const std::string& city)
{

}