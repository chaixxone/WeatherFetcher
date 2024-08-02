#include "weather_mapping.hpp"

WeatherMapper::WeatherMapper(const std::unordered_map<std::string, std::string>& config)
{ 
	std::unique_ptr<DB_Manager> dbman = std::make_unique<DB_Manager>(config);
	m_SECRET_API_KEY = dbman->GetKey();
}

static std::string getCurrent(const std::string& type)
{
	auto time_point_now = std::chrono::system_clock::now();
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(time_point_now);

	std::tm now_tm;
	localtime_s(&now_tm, &now_time_t);

	std::ostringstream type_stream;

	if (type == "date")
	{
		type_stream << std::put_time(&now_tm, "%Y-%m-%d");
	}
	else if (type == "time")
	{
		type_stream << std::put_time(&now_tm, "%H:%M:%S");
	}

	return type_stream.str();
}

std::string WeatherMapper::FetchWeatherData(const std::string& city)
{
	std::stringstream request;
	request << "https://api.openweathermap.org/data/2.5/weather?q=" << city << "&appid=" << m_SECRET_API_KEY;
	std::string getRequest = request.str();

	cpr::Response response = cpr::Get(cpr::Url(getRequest));	
	json jsonResponse;

	if (response.status_code == 200)
	{
		jsonResponse = std::move(json::parse(response.text));
	}
	else
	{
		throw std::runtime_error("response is not 200");
	}

	std::string weatherType = jsonResponse["weather"][0]["description"];
	short temperature = jsonResponse["main"]["temp"] - 273;

	json receivedData;
	receivedData["city"] = city;
	receivedData["date"] = getCurrent("date");
	receivedData["temp"] = temperature;
	receivedData["time"] = getCurrent("time");
	receivedData["weather"] = weatherType;	

	return receivedData.dump();
}