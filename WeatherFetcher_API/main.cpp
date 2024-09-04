#include "Z_API.hpp"
#include <memory>

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        std::cerr << "Usage:\n\tapi <db_host> <user> <password_file> <schema>" << std::endl;
        return 1;
    }

    std::string serverhost = "tcp://0.0.0.0:5555";

    try
    {
        std::unordered_map<std::string, std::string> config;
        config["host"] = argv[1];
        config["user"] = argv[2];
        config["password_file"] = argv[3];
        config["schema"] = argv[4];

        auto api = std::make_unique<Z_API>(serverhost, config);
        api->Run();
    }
    catch (const sql::SQLException& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (const zmq::error_t& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}