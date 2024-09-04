#include "ZClient.hpp"

using namespace std::chrono;

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

ZClient::ZClient(const std::string& host) : _context(1), _socket(_context, zmq::socket_type::req)
{
	_socket.connect(host);
	_authorise();
}

std::string ZClient::MakeRequest(const std::string& typeRequest, const std::string& data)
{
	zmq::multipart_t request;
	request.addstr(_token);
	request.addstr(typeRequest);
	request.addstr(data);
	request.send(_socket);

	zmq::message_t reply;
	_socket.recv(reply);
	auto response = reply.to_string();

	if (response == "Authentication required or token invalid")
	{
		auto resp = _authorise();
		return resp == "Authentication successful" ? MakeRequest(typeRequest, data) : "died";
	}

	return response;
}