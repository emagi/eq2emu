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

#include "HTTPSClient.h"
#include "PeerManager.h"

#include "../net.h"
#include "../../common/Log.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/steady_timer.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace boost_net = boost::asio;              // From <boost/asio.hpp>
extern NetConnection net;
extern PeerManager peer_manager;
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string base64_encode(const std::string& input) {
	std::string encoded_string;
	unsigned char const* bytes_to_encode = reinterpret_cast<const unsigned char*>(input.c_str());
	size_t in_len = input.size();
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				encoded_string += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			encoded_string += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			encoded_string += '=';
	}

	return encoded_string;
}

HTTPSClient::HTTPSClient(const std::string& certFile, const std::string& keyFile)
	: certFile(certFile), keyFile(keyFile) {}

std::shared_ptr<boost::asio::ssl::context> HTTPSClient::createSSLContext() {
	auto sslCtx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13_client);
	sslCtx->set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);
	sslCtx->use_certificate_file(certFile, boost::asio::ssl::context::pem);
	sslCtx->use_private_key_file(keyFile, boost::asio::ssl::context::pem);
	sslCtx->set_verify_mode(ssl::verify_peer);
	sslCtx->set_default_verify_paths();
	return sslCtx;
}

void HTTPSClient::parseAndStoreCookies(const http::response<http::string_body>& res) {
	if (res.count(http::field::set_cookie)) {
		std::istringstream stream(res[http::field::set_cookie].to_string());
		std::string token;

		// Parse "Set-Cookie" field for name-value pairs
		while (std::getline(stream, token, ';')) {
			auto pos = token.find('=');
			if (pos != std::string::npos) {
				std::string name = token.substr(0, pos);
				std::string value = token.substr(pos + 1);
				cookies[name] = value;  // Store each cookie
			}
		}
	}
}

std::string HTTPSClient::buildCookieHeader() const {
	std::string cookieHeader;
	for (const auto& [name, value] : cookies) {
		cookieHeader += name + "=" + value;
	}
	return cookieHeader;
}

std::string HTTPSClient::sendRequest(const std::string& server, const std::string& port, const std::string& target) {
	try {
		boost::asio::io_context ioContext;

		// SSL and TCP setup
		auto sslCtx = createSSLContext();
		auto stream = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(ioContext, *sslCtx);
		auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(ioContext);
		auto results = resolver->resolve(server, port);

		// Persistent objects to manage response, request, and buffer
		auto res = std::make_shared<http::response<http::string_body>>();
		auto buffer = std::make_shared<boost::beast::flat_buffer>();
		auto req = std::make_shared<http::request<http::string_body>>(http::verb::get, target, 11);

		// SNI hostname (required for many hosts)
		if (!SSL_set_tlsext_host_name(stream->native_handle(), server.c_str())) {
			throw boost::beast::system_error(
				boost::beast::error_code(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()));
		}

		// Prepare request headers
		req->set(http::field::host, server);
		req->set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req->set(boost::beast::http::field::connection, "close");
		req->set(http::field::content_type, "application/json");
		if (!cookies.empty()) {
			req->set(http::field::cookie, buildCookieHeader());
		}
		else {
			std::string credentials = net.GetCmdUser() + ":" + net.GetCmdPassword();
			std::string encodedCredentials = base64_encode(credentials);
			req->set(http::field::authorization, "Basic " + encodedCredentials);
		}

		// Step 1: Asynchronous connect with timeout
		auto connect_timer = std::make_shared<boost::asio::steady_timer>(ioContext);
		connect_timer->expires_after(std::chrono::seconds(2));

		connect_timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
			if (!ec) {
				stream->lowest_layer().cancel();  // Cancel operation on timeout
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: Connect Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
				peer_manager.SetPeerErrorState(server, port);
			}
			});

		auto timer = std::make_shared<boost::asio::steady_timer>(ioContext, std::chrono::seconds(2));
		boost::asio::async_connect(stream->lowest_layer(), results,
			[stream, connect_timer, req, buffer, res, timer, server, port, target](boost::system::error_code ec, const auto&) {
				connect_timer->cancel();
				if (ec) {
					LogWrite(PEERING__ERROR, 0, "Peering", "%s: Connect Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
					peer_manager.SetPeerErrorState(server, port);
					return;
				}

				// Step 2: Asynchronous handshake with timeout
				timer->expires_after(std::chrono::seconds(2));

				timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
					if (!ec) {
						stream->lowest_layer().cancel();
						LogWrite(PEERING__ERROR, 0, "Peering", "%s: Handshake Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
						peer_manager.SetPeerErrorState(server, port);
					}
					});

				stream->async_handshake(boost::asio::ssl::stream_base::client,
					[stream, timer, req, buffer, res, server, port, target](boost::system::error_code ec) {
						timer->cancel();
						if (ec) {
							LogWrite(PEERING__ERROR, 0, "Peering", "%s: Handshake Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
							peer_manager.SetPeerErrorState(server, port);
							return;
						}

						// Step 3: Asynchronous write request
						timer->expires_after(std::chrono::seconds(2));

						timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
							if (!ec) {
								stream->lowest_layer().cancel();
								LogWrite(PEERING__ERROR, 0, "Peering", "%s: Write Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
								peer_manager.SetPeerErrorState(server, port);
							}
							});

						http::async_write(*stream, *req,
							[stream, buffer, res, timer, server, port, target](boost::system::error_code ec, std::size_t) {
								timer->cancel();
								if (ec) {
									LogWrite(PEERING__ERROR, 0, "Peering", "%s: Write Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
									peer_manager.SetPeerErrorState(server, port);
									return;
								}

								// Step 4: Asynchronous read response
								timer->expires_after(std::chrono::seconds(2));

								timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
									if (!ec) {
										stream->lowest_layer().cancel();
										LogWrite(PEERING__ERROR, 0, "Peering", "%s: Read Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
										peer_manager.SetPeerErrorState(server, port);
									}
									});

								http::async_read(*stream, *buffer, *res,
									[stream, timer, res, server, port, target](boost::system::error_code ec, std::size_t) {
										timer->cancel();
										if (ec) {
											LogWrite(PEERING__ERROR, 0, "Peering", "%s: Read Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
											peer_manager.SetPeerErrorState(server, port);
											return;
										}

										// Step 5: Shutdown the stream
										stream->async_shutdown([stream, server, port](boost::system::error_code ec) {
											if (ec && ec != boost::asio::error::eof) {
												// ignore these
												//std::cerr << "Shutdown error: " << ec.message() << std::endl;
											}
											});
									});
							});
					});
			});

		ioContext.run();

		// Store cookies from the response
		if (res->base().count(http::field::set_cookie) > 0) {
			auto set_cookie_value = res->base()[http::field::set_cookie].to_string();
			std::istringstream stream(set_cookie_value);
			std::string token;

			// Parse "Set-Cookie" field for name-value pairs
			while (std::getline(stream, token, ';')) {
				auto pos = token.find('=');
				if (pos != std::string::npos) {
					std::string name = token.substr(0, pos);
					std::string value = token.substr(pos + 1);
					cookies[name] = value;  // Store each cookie
				}
			}
		}

		if (res->body() == "Unauthorized") {
			cookies.clear();
		}

		// Return the response body, if available
		return res->body();
	}
	catch (const std::exception& e) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Request Error %s for %s:%s/%s", __FUNCTION__, e.what() ? e.what() : "??", server.c_str(), port.c_str(), target.c_str());
		return {};
	}
}

std::string HTTPSClient::sendPostRequest(const std::string& server, const std::string& port, const std::string& target, const std::string& jsonPayload) {
	try {
		boost::asio::io_context ioContext;

		// SSL and TCP setup
		auto sslCtx = createSSLContext();
		auto stream = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(ioContext, *sslCtx);
		auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(ioContext);
		auto results = resolver->resolve(server, port);

		// Persistent objects to manage response, request, and buffer
		auto res = std::make_shared<http::response<http::string_body>>();
		auto buffer = std::make_shared<boost::beast::flat_buffer>();
		auto req = std::make_shared<http::request<http::string_body>>(http::verb::post, target, 11);

		// SNI hostname (required for many hosts)
		if (!SSL_set_tlsext_host_name(stream->native_handle(), server.c_str())) {
			throw boost::beast::system_error(
				boost::beast::error_code(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()));
		}

		// Prepare HTTP POST request with JSON payload
		req->set(http::field::host, server);
		req->set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req->set(boost::beast::http::field::connection, "close");
		req->set(http::field::content_type, "application/json");
		if (!cookies.empty()) {
			req->set(http::field::cookie, buildCookieHeader());
		}
		else {
			std::string credentials = net.GetCmdUser() + ":" + net.GetCmdPassword();
			std::string encodedCredentials = base64_encode(credentials);
			req->set(http::field::authorization, "Basic " + encodedCredentials);
		}

		req->body() = jsonPayload;
		req->prepare_payload();

		// Step 1: Asynchronous connect with timeout
		auto connect_timer = std::make_shared<boost::asio::steady_timer>(ioContext);
		connect_timer->expires_after(std::chrono::seconds(2));

		connect_timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
			if (!ec) {
				stream->lowest_layer().cancel();  // Cancel operation on timeout
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: Connect Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
				peer_manager.SetPeerErrorState(server, port);
			}
			});

		auto timer = std::make_shared<boost::asio::steady_timer>(ioContext, std::chrono::seconds(2));
		boost::asio::async_connect(stream->lowest_layer(), results,
			[stream, connect_timer, req, buffer, res, timer, server, port, target](boost::system::error_code ec, const auto&) {
				connect_timer->cancel();
				if (ec) {
					LogWrite(PEERING__ERROR, 0, "Peering", "%s: Connect Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
					peer_manager.SetPeerErrorState(server, port);
					return;
				}

				// Step 2: Asynchronous handshake with timeout
				timer->expires_after(std::chrono::seconds(2));

				timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
					if (!ec) {
						stream->lowest_layer().cancel();
						LogWrite(PEERING__ERROR, 0, "Peering", "%s: Handshake Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
						peer_manager.SetPeerErrorState(server, port);
					}
					});

				stream->async_handshake(boost::asio::ssl::stream_base::client,
					[stream, timer, req, buffer, res, server, port, target](boost::system::error_code ec) {
						timer->cancel();
						if (ec) {
							LogWrite(PEERING__ERROR, 0, "Peering", "%s: Handshake Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
							peer_manager.SetPeerErrorState(server, port);
							return;
						}

						// Step 3: Asynchronous write request
						timer->expires_after(std::chrono::seconds(2));

						timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
							if (!ec) {
								stream->lowest_layer().cancel();
								LogWrite(PEERING__ERROR, 0, "Peering", "%s: Write Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
								peer_manager.SetPeerErrorState(server, port);
							}
							});

						http::async_write(*stream, *req,
							[stream, buffer, res, timer, server, port, target](boost::system::error_code ec, std::size_t) {
								timer->cancel();
								if (ec) {
									LogWrite(PEERING__ERROR, 0, "Peering", "%s: Write Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
									peer_manager.SetPeerErrorState(server, port);
									return;
								}

								// Step 4: Asynchronous read response
								timer->expires_after(std::chrono::seconds(2));

								timer->async_wait([stream, server, port, target](boost::system::error_code ec) {
									if (!ec) {
										stream->lowest_layer().cancel();
										LogWrite(PEERING__ERROR, 0, "Peering", "%s: Read Timeout for %s:%s/%s", __FUNCTION__, server.c_str(), port.c_str(), target.c_str());
										peer_manager.SetPeerErrorState(server, port);
									}
									});

								http::async_read(*stream, *buffer, *res,
									[stream, timer, res, server, port, target](boost::system::error_code ec, std::size_t) {
										timer->cancel();
										if (ec) {
											LogWrite(PEERING__ERROR, 0, "Peering", "%s: Read Error %s for %s:%s/%s", __FUNCTION__, ec.message().c_str(), server.c_str(), port.c_str(), target.c_str());
											peer_manager.SetPeerErrorState(server, port);
											return;
										}

										// Step 5: Shutdown the stream
										stream->async_shutdown([stream, server, port](boost::system::error_code ec) {
											if (ec && ec != boost::asio::error::eof) {
												// ignore these
												//std::cerr << "Shutdown error: " << ec.message() << std::endl;
											}
											});
									});
							});
					});
			});

		ioContext.run();

		// Store cookies from the response
		if (res->base().count(http::field::set_cookie) > 0) {
			auto set_cookie_value = res->base()[http::field::set_cookie].to_string();
			std::istringstream stream(set_cookie_value);
			std::string token;

			// Parse "Set-Cookie" field for name-value pairs
			while (std::getline(stream, token, ';')) {
				auto pos = token.find('=');
				if (pos != std::string::npos) {
					std::string name = token.substr(0, pos);
					std::string value = token.substr(pos + 1);
					cookies[name] = value;  // Store each cookie
				}
			}
		}

		if (res->body() == "Unauthorized") {
			cookies.clear();
		}

		// Return the response body, if available
		return res->body();
	}
	catch (const std::exception& e) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Request Error %s for %s:%s/%s", __FUNCTION__, e.what() ? e.what() : "??", server.c_str(), port.c_str(), target.c_str());
		return {};
	}
}