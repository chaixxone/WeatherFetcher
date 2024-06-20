#include "weather_mapping.hpp"

WeatherMapper::WeatherMapper() : m_io_context(), m_socket(m_io_context), m_resolver(m_io_context) 
{ 
	std::unique_ptr<DB_Manager> dbman;
	m_SECRET_API_KEY = dbman->GetKey();
}

void WeatherMapper::FetchWeatherData(const std::string& city)
{

}