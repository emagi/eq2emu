/*
EQ2Emu:  Everquest II Server Emulator
Copyright (C) 2007-2025  EQ2Emu Development Team (https://www.eq2emu.com)

This file is part of EQ2Emu.

EQ2Emu is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EQ2Emu is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EQ2Emu.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HTTPSCLIENT_H
#define HTTPSCLIENT_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <unordered_map>

namespace ssl = boost::asio::ssl;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

class HTTPSClient {
public:
	HTTPSClient(const std::string& certFile, const std::string& keyFile);

	// Send a request with stored cookies and return response as string
	std::string sendRequest(const std::string& server, const std::string& port, const std::string& target);

	// Send a POST request with a JSON payload and return response as string
	std::string sendPostRequest(const std::string& server, const std::string& port, const std::string& target, const std::string& jsonPayload);

	std::string getServer() const { return server; }
	std::string getPort() const { return port; }

private:
	std::unordered_map<std::string, std::string> cookies;
    std::shared_ptr<boost::asio::ssl::context> sslCtx;
	
	std::string certFile;
	std::string keyFile;
	std::string server;
	std::string port;

	void parseAndStoreCookies(const http::response<http::string_body>& res);
	std::shared_ptr<boost::asio::ssl::context> createSSLContext();  // New helper function
	std::string buildCookieHeader() const;
};

#endif // HTTPSCLIENT_H
