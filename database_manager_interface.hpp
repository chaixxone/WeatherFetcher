#include "weather_data.hpp"
#include <memory>

class IDB_Manager
{
public:
	virtual void Write(std::unique_ptr<WeatherData> weatherData) = 0;
	virtual WeatherData ReadLast(const std::string& city) = 0;
	virtual std::string GetKey() = 0;
	virtual void LoadImage(const std::string& imagePath, const std::string& name) = 0;
	virtual std::string GetImageData(const std::string& imageName) = 0;
};