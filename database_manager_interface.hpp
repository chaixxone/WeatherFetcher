#include "weather_data.hpp"
#include <memory>

class IDB_Manager
{
public:
	virtual void Write(std::unique_ptr<WeatherData> weatherData) = 0;
	virtual WeatherData ReadLast() = 0;
	virtual std::string GetKey() = 0;
};