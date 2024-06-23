#include "weather_mapping.hpp"

WeatherMapper::WeatherMapper() : m_io_context(), m_socket(m_io_context), m_resolver(m_io_context)
{
	std::unique_ptr<DB_Manager> dbman = std::make_unique<DB_Manager>();
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

WeatherData WeatherMapper::FetchWeatherData(const std::string& city)
{
	ip::tcp::resolver::query query("api.openweathermap.org", "80");
	ip::tcp::resolver::iterator it = m_resolver.resolve(query);
	ip::tcp::endpoint ep(*it);
	m_socket.connect(ep);

	std::string getRequest = "GET /data/2.5/weather?q=" + city + "&appid=" + m_SECRET_API_KEY + " HTTP/1.1\r\n";
	getRequest += "Host: api.openweathermap.org\r\n";
	getRequest += "Connection: close\r\n\r\n";

	m_socket.send(buffer(getRequest));

	streambuf api_response;

	try
	{
		read_until(m_socket, api_response, "\r\n");
	}
	catch (const boost::system::system_error& e)
	{
		std::cerr << "Boost system error: " << e.what() << std::endl;
	}

	std::istream responseStream(&api_response);
	std::string httpVersion;
	responseStream >> httpVersion;
	unsigned short statusCode;
	responseStream >> statusCode;
	std::string statusMessage;

	std::getline(responseStream, statusMessage);

	if (!responseStream || httpVersion.substr(0, 5) != "HTTP/")
	{
		throw std::runtime_error("Invalid response");
	}

	if (statusCode != 200)
	{
		throw std::runtime_error("Response returned with status code " + std::to_string(statusCode));
	}

	boost::system::error_code error;

	while (boost::asio::read(m_socket, api_response, boost::asio::transfer_at_least(1), error))
	{
		// read while there is some data
	}

	if (error != boost::asio::error::eof)
	{
		throw boost::system::system_error(error);
	}

	std::string responseData = boost::asio::buffer_cast<const char*>(api_response.data());

	size_t jsonDataStartAt = responseData.find_first_of('{');

	if (jsonDataStartAt == std::string::npos)
	{
		throw std::runtime_error("Failed to find JSON start");
	}

	std::string jsonRealData = responseData.substr(jsonDataStartAt);

	json jsonResponse = json::parse(jsonRealData);

	std::string weatherType = jsonResponse["weather"][0]["description"];

	short temperature = jsonResponse["main"]["temp"] - 273;

	std::unique_ptr<WeatherData> weatherData = std::make_unique<WeatherData>();
	weatherData->City = city;
	weatherData->Date = getCurrent("date");
	weatherData->Temperature = std::to_string(temperature);
	weatherData->Time = getCurrent("time");
	weatherData->WeatherType = weatherType;

	return *weatherData;
}
