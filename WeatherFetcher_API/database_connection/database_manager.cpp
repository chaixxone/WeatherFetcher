#include "database_manager.hpp"

std::string DB_Manager::_getPassword(const std::string& pathToFile)
{
    std::ifstream file(pathToFile);

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

DB_Manager::DB_Manager(const std::unordered_map<std::string, std::string>& config) 
    : m_server(config.at("host")), m_username(config.at("user")), m_password(_getPassword(config.at("password_file")))
{ 
    m_driver = get_driver_instance();
    m_connection = std::unique_ptr<sql::Connection>(m_driver->connect(m_server, m_username, m_password));
    m_connection->setSchema(config.at("schema"));
}

void DB_Manager::Write(std::unique_ptr<WeatherData> weatherData)
{
    try
    {
        auto pstmt = std::unique_ptr<sql::PreparedStatement>(
            m_connection->prepareStatement(
                "INSERT INTO snaps (snap_date, snap_time, city, temperature, weather_type) VALUES (?, ?, ?, ?, ?)"
            )
        );

        pstmt->setString(1, weatherData->Date);
        pstmt->setString(2, weatherData->Time);
        pstmt->setString(3, weatherData->City);
        pstmt->setDouble(4, std::stoi(weatherData->Temperature));
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
        auto pstmt = std::unique_ptr<sql::PreparedStatement>(
            m_connection->prepareStatement(
                "SELECT * FROM snaps WHERE city=? ORDER BY id DESC LIMIT 1"
            )
        );
        pstmt->setString(1, city);
        auto result = std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());
        
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
        auto pstmt = std::unique_ptr<sql::PreparedStatement>(m_connection->prepareStatement("SELECT * FROM secret"));
        auto result = std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());

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

void DB_Manager::LoadImageData(const std::string& imagePath, const std::string& name)
{
    try 
    {
        std::ifstream file(imagePath, std::ios::binary);

        if (!file.is_open()) 
        {
            throw std::runtime_error("Failed to open image file");
        }
        
        auto pstmt = std::unique_ptr<sql::PreparedStatement>(m_connection->prepareStatement("INSERT INTO pictures (weather, picture) VALUES (?, ?)"));

        pstmt->setString(1, name);
        pstmt->setBlob(2, &file);
        pstmt->execute();

        file.close();

        std::cout << "Image uploaded successfully" << std::endl;
    }
    catch (sql::SQLException& e) 
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        std::cerr << "Error Code: " << e.getErrorCode() << std::endl;
    }
    catch (std::exception& e) 
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

std::string DB_Manager::GetImageData(const std::string& imageName)
{
    try
    {
        auto pstmt = std::unique_ptr<sql::PreparedStatement>(m_connection->prepareStatement("SELECT picture FROM pictures WHERE weather=?"));
        pstmt->setString(1, imageName);
        auto result = std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());

        if (result->next())
        {
            std::unique_ptr<std::istream> pic(result->getBlob("picture"));

            if (pic->peek() == std::istream::traits_type::eof())
            {
                throw std::runtime_error("Retrieved BLOB data is empty");
            }

            std::stringstream buffer;
            buffer << pic->rdbuf();
            std::string data = buffer.str();

            return data;
        }

        return "None";
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        std::cerr << "Error Code: " << e.getErrorCode() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}