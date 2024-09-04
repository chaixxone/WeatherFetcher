#include <string>
#include <chrono>

using namespace std::chrono;

class TokenInfo
{
public:
    TokenInfo() : _token(""), _expirationTime(system_clock::now()) {}

    bool IsExpired() const
    {
        return _expirationTime <= system_clock::now();
    }
    void SetInfo(const std::string& token, const system_clock::time_point& expTime)
    {
        _token = token;
        _expirationTime = expTime;
    }
private:
    std::string _token;
    system_clock::time_point _expirationTime;
};