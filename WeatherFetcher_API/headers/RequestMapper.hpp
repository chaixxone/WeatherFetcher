#include <unordered_map>

class RequestsMapper
{
protected:
    enum class Requests
    {
        Authorisation,
        GetFileData,
        InsertImage,
        GetImage,
        GetAPIKey,
        InsertWeatherData,
        GetWeatherData,
        Unknown
    };

    static Requests StringToEnum(const std::string& request)
    {
        static const std::unordered_map<std::string, Requests> requestMap = {
            { "auth", Requests::Authorisation },
            { "get_data_from_file", Requests::GetFileData },
            { "insert_weather_data", Requests::InsertWeatherData },
            { "get_weather_data", Requests::GetWeatherData },
            { "get_image", Requests::GetImage },
            { "insert_image", Requests::InsertImage },
            { "get_api_key", Requests::GetAPIKey }
        };
        auto it = requestMap.find(request);
        if (it != requestMap.end())
        {
            return it->second;
        }
        return Requests::Unknown;
    }
};