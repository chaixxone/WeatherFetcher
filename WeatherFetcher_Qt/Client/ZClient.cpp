#include "ZClient.hpp"

using namespace std::chrono;

ZClient::ZClient() : _context(1), _socket(_context, zmq::socket_type::req)
{
	_socket.set(zmq::sockopt::linger, 2000);
}

void ZClient::_setToken()
{
	auto expireTime = system_clock::now() + seconds(10);

	auto token = jwt::create()
		.set_issuer("auth0")
		.set_payload_claim("sample", jwt::claim(std::string("authenticated_user")))
		.set_expires_at(expireTime)
		.sign(jwt::algorithm::hs256{ "secret" });

	_token = token;
}

std::string ZClient::_authorise()
{
	_setToken();
	auto authoriseReply = MakeRequest("auth", "");
	return authoriseReply;
}

void ZClient::DestroyClientWork()
{
	_context.shutdown();
}

void ZClient::Connect(const std::string& host)
{
	_socket.connect(host);
	_authorise();
}

std::string ZClient::MakeRequest(const std::string& typeRequest, const std::string& data)
{
	zmq::multipart_t request;
	zmq::message_t reply;

	std::unique_lock<std::mutex> lock(_mutex);

	try
	{
		request.addstr(_token);
		request.addstr(typeRequest);
		request.addstr(data);
		request.send(_socket);

		zmq::recv_result_t res = _socket.recv(reply);

		if (!res.has_value()) return "Received empty reply";
	}
	catch (const zmq::error_t& e)
	{
		if (e.num() == ETERM)
		{			
			throw std::runtime_error("Client is destroyed");
		}
	}

	std::string response = reply.to_string();

	if (response == "Authentication required or token invalid")
	{
		std::string authorizeResponse = _authorise();
		return authorizeResponse == "Authentication successful" ? MakeRequest(typeRequest, data) : "died";
	}

	return response;
}