#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <mutex>

#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "json.hpp"
#include <jwt-cpp/jwt.h>

class ZClient
{
public:
	ZClient();
	std::string MakeRequest(const std::string& typeRequest, const std::string& data);
	void Connect(const std::string& host);
	void DestroyClientWork();

private:
	std::recursive_mutex _mutex;
	zmq::context_t _context;
	zmq::socket_t _socket;
	std::string _token;
	std::string _authorise();
	void _setToken();
};