#include "stdafx.hpp"
#include "TokenInfo.hpp"
#include "RequestMapper.hpp"
#include "database_manager.hpp"

using namespace std::chrono;

class Z_API : RequestsMapper
{
public:
    Z_API(const std::string& host, const std::unordered_map<std::string, std::string>& config);
    void Run();

private:
    zmq::context_t _context;
    zmq::socket_t _socket;
    TokenInfo tokenOfClient;
    std::unique_ptr<DB_Manager> m_db;

    bool _verifyJWT(const std::string& token);
    void _handleRequests(const Requests& requestEnum, const std::string& requestData, const std::string& token);    
};