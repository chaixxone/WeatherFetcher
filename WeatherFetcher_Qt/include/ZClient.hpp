#pragma once
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <iostream>
#include <fstream>
#include "json.hpp"
#include <thread>
#include <jwt-cpp/jwt.h>
#include <chrono>

class ZClient
{
public:
	ZClient(const std::string& host);
	std::string MakeRequest(const std::string& typeRequest, const std::string& data);

private:
	zmq::context_t _context;
	zmq::socket_t _socket;
	std::string _token;
	std::string _authorise();
	void _setToken();
};