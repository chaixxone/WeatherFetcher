#include "database_manager.hpp"

std::string DB_Manager::_getPassword(const std::string& pathToFile)
{
    std::ifstream file("password.txt");

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open password file");
    }

    std::string password;

    if (!std::getline(file, password))
    {
        throw std::runtime_error("Failed to read password from file");
    }

    file.close();

    return password;
}

DB_Manager::DB_Manager() : m_server("tcp://localhost:3306"), m_username("root"), m_password(_getPassword("password.txt")) { }

void DB_Manager::Write(std::unique_ptr<WeatherData> weatherData)
{
    try
    {
        m_driver = get_driver_instance();
        std::unique_ptr<sql::Connection> connection;
        std::unique_ptr<sql::PreparedStatement> pstmt;

        connection = std::unique_ptr<sql::Connection>(m_driver->connect(m_server, m_username, m_password));
        connection->setSchema("weather");

        pstmt = std::unique_ptr<sql::PreparedStatement>(
            connection->prepareStatement(
                "INSERT INTO snapshots (snap_date, snap_time, city, temperature, weather_type) VALUES (?, ?, ?, ?, ?)"
            )
        );

        pstmt->setString(1, weatherData->Date);
        pstmt->setString(2, weatherData->Time);
        pstmt->setString(3, weatherData->City);
        pstmt->setDouble(4, std::stod(weatherData->Temperature));
        pstmt->setString(5, weatherData->WeatherType);

        pstmt->executeUpdate();
    }
    catch (const sql::SQLException& e)
    {
        std::ofstream logFile("log.txt", std::ios_base::app);
        logFile << "SQL Exception: " << e.what() << std::endl;
    }
}

WeatherData DB_Manager::ReadLast(const std::string& city)
{
    WeatherData lastWDSnapshot;

    try
    {
        m_driver = get_driver_instance();
        std::unique_ptr<sql::Connection> connection;
        std::unique_ptr<sql::PreparedStatement> pstmt;
        std::unique_ptr<sql::ResultSet> result;

        connection = std::unique_ptr<sql::Connection>(m_driver->connect(m_server, m_username, m_password));        
        connection->setSchema("weather");

        pstmt = std::unique_ptr<sql::PreparedStatement>(connection->prepareStatement("SELECT * FROM snapshots WHERE city='" + city + "' ORDER BY id DESC LIMIT 1; "));
        result = std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());
        
        if (result->next())
        {
            lastWDSnapshot.Date = result->getString("snap_date");
            lastWDSnapshot.Time = result->getString("snap_time");
            lastWDSnapshot.City = city;
            lastWDSnapshot.Temperature = std::to_string(result->getDouble("temperature"));
            lastWDSnapshot.WeatherType = result->getString("weather_type");
        }
    }
    catch (const sql::SQLException& e)
    {
        std::ofstream logFile("log.txt", std::ios_base::app);
        logFile << "SQL Exception: " << e.what() << std::endl;
    }

    return lastWDSnapshot;
}

std::string DB_Manager::GetKey()
{
    std::string apiKey;

    try
    {
        m_driver = get_driver_instance();
        std::unique_ptr<sql::Connection> connection;
        std::unique_ptr<sql::PreparedStatement> pstmt;
        std::unique_ptr<sql::ResultSet> result;

        connection = std::unique_ptr<sql::Connection>(m_driver->connect(m_server, m_username, m_password));
        connection->setSchema("weather");

        pstmt = std::unique_ptr<sql::PreparedStatement>(connection->prepareStatement("SELECT * FROM secret;"));
        result = std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());

        if (result->next())
        {
            apiKey = result->getString("api_key");
        }
    }
    catch (const sql::SQLException& e)
    {
        std::ofstream logFile("log.txt", std::ios_base::app);
        logFile << "SQL Exception: " << e.what() << std::endl;
    }

    return apiKey;
}
