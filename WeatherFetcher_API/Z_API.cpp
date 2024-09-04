#include "Z_API.hpp"
#include <filesystem>

Z_API::Z_API(const std::string& host, const std::unordered_map<std::string, std::string>& config) 
    : _context(1), _socket(_context, zmq::socket_type::rep), tokenOfClient(), m_db(new DB_Manager(config))
{
    _socket.bind(host);
}

void Z_API::Run()
{
    while (true)
    {
        try
        {
            zmq::multipart_t request;
            request.recv(_socket);

            if (request.empty())
            {
                std::cerr << "Received empty request" << std::endl;
                continue;
            }

            std::string token = request.popstr();
            std::string requestStr = request.popstr();
            std::string additionalData = request.popstr();

            Requests requestEnum = StringToEnum(requestStr);
            _handleRequests(requestEnum, additionalData, token);
        }
        catch (const zmq::error_t& e)
        {
            std::cerr << "ZeroMQ error: " << e.what() << std::endl;
        }
        catch (const nlohmann::json::parse_error& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

bool Z_API::_verifyJWT(const std::string& token)
{
    try
    {
        auto decoded_token = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ "secret" })
            .with_issuer("auth0");
        verifier.verify(decoded_token);

        auto exp = decoded_token.get_expires_at();
        auto now = std::chrono::system_clock::now();

        if (exp < now)
        {
            throw std::runtime_error("Token has expired");
        }

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "JWT verification failed: " << e.what() << std::endl;
        return false;
    }
}

void Z_API::_handleRequests(const Requests& requestEnum, const std::string& requestData, const std::string& token)
{
    if (requestEnum == Requests::Authorisation && _verifyJWT(token))
    {
        auto now = std::chrono::system_clock::now();
        auto exp_time = now + std::chrono::seconds(10);
        tokenOfClient.SetInfo(token, exp_time);
        _socket.send(zmq::str_buffer("Authentication successful"));
    }
    else if (!tokenOfClient.IsExpired())
    {
        switch (requestEnum)
        {
        case Requests::GetAPIKey:
        {
            auto APIKey = m_db->GetKey();
            _socket.send(zmq::buffer(APIKey));
            std::cout << "new connection!" << std::endl;
            break;
        }
        case Requests::InsertImage:
        {
            std::filesystem::path filepath(requestData);                
            std::string weatherTypeName = filepath.stem().string();
            m_db->LoadImageData(requestData, weatherTypeName);
            _socket.send(zmq::str_buffer("successfully loaded file"));
            break;
        }
        case Requests::GetImage:
        {
            auto imageDataStr = m_db->GetImageData(requestData);
            zmq::message_t imageData(imageDataStr);
            _socket.send(imageData);
            break;
        }
        case Requests::GetWeatherData:
        {            
            auto snapshot = m_db->ReadLast(requestData);    

            std::string response = "no data";

            if (!snapshot.City.empty())
            {
                nlohmann::json data;
                data["city"] = snapshot.City;
                data["date"] = snapshot.Date;
                data["temp"] = snapshot.Temperature;
                data["time"] = snapshot.Time;
                data["weather"] = snapshot.WeatherType;

                response = data.dump();
            }

            _socket.send(zmq::buffer(response));

            break;
        }
        case Requests::InsertWeatherData:
        {
            nlohmann::json data = nlohmann::json::parse(requestData);
            
            std::unique_ptr<WeatherData> incomingData(new WeatherData);
            incomingData->City = data["city"];
            incomingData->Date = data["date"];
            short temp = data["temp"];
            incomingData->Temperature = std::to_string(temp);
            incomingData->Time = data["time"];
            incomingData->WeatherType = data["weather"];

            m_db->Write(std::move(incomingData));

            _socket.send(zmq::str_buffer("successfully sent"));
            break;
        }
        case Requests::Unknown:
            _socket.send(zmq::str_buffer("Invalid request"));
            break;
        }
    }
    else
    {
        _socket.send(zmq::str_buffer("Authentication required or token invalid"));
    }
}
