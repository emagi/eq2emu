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

HTTPSClient::HTTPSClient(const std::string& certFile,
                         const std::string& keyFile)
  : certFile(certFile)
  , keyFile(keyFile)
  , ioc_()
  , workGuard_(boost::asio::make_work_guard(ioc_))   // ◀︎ keep run() from returning
  , sslCtx(createSSLContext())
  , pool_(ioc_, *sslCtx)                // pass sslCtx here
{
  // fire up the background I/O thread
  runner_ = std::thread([&]{ ioc_.run(); });
}

HTTPSClient::~HTTPSClient() {
  workGuard_.reset();
  ioc_.stop();
  runner_.join();
}

std::shared_ptr<boost::asio::ssl::context> HTTPSClient::createSSLContext() {
	auto sslCtx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13_client);
	sslCtx->set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);
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

std::string HTTPSClient::sendRequest(
	const std::string& server,
	const std::string& port,
	const std::string& target) {
  // promise/future to block until async completes
  std::promise<std::pair<boost::system::error_code, std::string>> p;
  auto f = p.get_future();

  // call the async overload
  sendRequest(server, port, target,
	[&p](boost::system::error_code ec, std::string body) {
	  p.set_value({ec, std::move(body)});
	});

  auto [ec, body] = f.get();
  if (ec) {
	LogWrite(PEERING__ERROR, 0, "Peering",
			 "%s: Request Error %s", __FUNCTION__, ec.message().c_str());
	return {};
  }
  return body;
}

// async GET
void HTTPSClient::sendRequest(
	const std::string& server,
	const std::string& port,
	const std::string& target,
	std::function<void(boost::system::error_code, std::string)> done)
{
  pool_.acquire(server, port,
	[this, server, port, target, done](auto ps, auto ec) {
	  if (ec) return done(ec, "");

	  auto req = std::make_shared<
		http::request<http::string_body>>(
		  http::verb::get, target, 11);

	  req->set(http::field::host, server);
	  req->set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
	  req->set(http::field::connection, "keep-alive");
	  if (!cookies.empty()) {
		req->set(http::field::cookie, buildCookieHeader());
	  } else {
		auto creds = net.GetCmdUser() + ":" + net.GetCmdPassword();
		req->set(http::field::authorization,
				 "Basic " + base64_encode(creds));
	  }

	  auto buffer = std::make_shared<boost::beast::flat_buffer>();
	  auto res    = std::make_shared<
		http::response<http::string_body>>();
	  auto write_timer   = std::make_shared<boost::asio::steady_timer>(ioc_);
	  auto read_timer    = std::make_shared<boost::asio::steady_timer>(ioc_);

	  write_timer->expires_after(std::chrono::seconds(2));
	  write_timer->async_wait([ps](auto ec){
		if (!ec) {
		  // cancel the write if it’s still pending
		  ps->stream.lowest_layer().cancel();
		}
	  });
	  // capture 'req' so it sticks around till write completes
	  http::async_write(ps->stream, *req,
		[this, ps, req, buffer, res, write_timer, read_timer, server, port, done]
		(boost::system::error_code ec, std::size_t) {
			write_timer->cancel();
			
		  if (ec) {
			// write failed—drop this connection entirely
			ps->stream.lowest_layer().close();
			return done(ec, "");
		  }
		  
		  read_timer->expires_after(std::chrono::seconds(5));
		  read_timer->async_wait([ps](auto ec){
			if (!ec) {
			  // cancel the read if it’s still pending
			  ps->stream.lowest_layer().cancel();
			}
		  });
		  
		  http::async_read(ps->stream, *buffer, *res,
			[this, ps, buffer, res, read_timer, server, port, done]
			(boost::system::error_code ec, std::size_t) {
				read_timer->cancel();
				
			  if (ec) {
				// read failed or timed out—drop it
				ps->stream.lowest_layer().close();
				return done(ec, "");
			  }
			  
			  pool_.release(server, port, ps);

				auto status = res->result();
				if (status == http::status::unauthorized) {
				  cookies.clear();  // clear out any bad cookies
				  return done({},
							  "Unauthorized");
				}
				if (status != http::status::ok) {
				  LogWrite(PEERING__ERROR, 0, "Peering",
						   "%s: HTTP error %u", __FUNCTION__, status);
				  return done(
					boost::system::error_code(
					  static_cast<int>(status),
					  boost::asio::error::get_ssl_category()
					),
					"");
				}
			  // cookie logic
			  if (res->base().count(http::field::set_cookie)) {
				auto hdr = res->base()[http::field::set_cookie]
							   .to_string();
				std::istringstream ss(hdr);
				std::string token;
				while (std::getline(ss, token, ';')) {
				  auto pos = token.find('=');
				  if (pos!=std::string::npos) {
					cookies[token.substr(0,pos)] =
					  token.substr(pos+1);
				  }
				}
			  }
			  if (res->body() == "Unauthorized")
				cookies.clear();

			  done({}, res->body());
			});
		});
	});
}


std::string HTTPSClient::sendPostRequest(
	const std::string& server,
	const std::string& port,
	const std::string& target,
	const std::string& jsonPayload) {
  std::promise<std::pair<boost::system::error_code, std::string>> p;
  auto f = p.get_future();

  // call the async version internally
  sendPostRequest(server, port, target, jsonPayload,
	[&p](boost::system::error_code ec, std::string body) {
	  p.set_value({ec, std::move(body)});
	});

  auto [ec, body] = f.get();
  if (ec) {
	LogWrite(PEERING__ERROR, 0, "Peering",
			 "%s: error %s", __FUNCTION__, ec.message().c_str());
	return {};
  }
  return body;
}

// async POST
void HTTPSClient::sendPostRequest(
	const std::string& server,
	const std::string& port,
	const std::string& target,
	const std::string& jsonPayload,
	std::function<void(boost::system::error_code, std::string)> done)
{
  pool_.acquire(server, port,
	[this, server, port, target, jsonPayload, done](auto ps, auto ec) {
	  if (ec) return done(ec, "");

	  // — heap-allocated POST req —
	  auto req = std::make_shared<
		http::request<http::string_body>>(
		  http::verb::post, target, 11);

	  req->set(http::field::host, server);
	  req->set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
	  req->set(http::field::connection, "keep-alive");
	  req->set(http::field::content_type,
			   "application/json");
	  if (!cookies.empty()) {
		req->set(http::field::cookie,
				 buildCookieHeader());
	  } else {
		auto creds = net.GetCmdUser() + ":" +
					 net.GetCmdPassword();
		req->set(http::field::authorization,
				 "Basic " + base64_encode(creds));
	  }

	  req->body() = jsonPayload;
	  req->prepare_payload();

	  auto buffer = std::make_shared<
		boost::beast::flat_buffer>();
	  auto res    = std::make_shared<
		http::response<http::string_body>>();
	  auto write_timer   = std::make_shared<boost::asio::steady_timer>(ioc_);
	  auto read_timer    = std::make_shared<boost::asio::steady_timer>(ioc_);

	  write_timer->expires_after(std::chrono::seconds(2));
	  write_timer->async_wait([ps](auto ec){
		if (!ec) {
		  // cancel the write if it’s still pending
		  ps->stream.lowest_layer().cancel();
		}
	  });
	  // keep 'req' alive until write finishes
	  http::async_write(ps->stream, *req,
		[this, ps, req, buffer, res, write_timer, read_timer, server, port, done]
		(boost::system::error_code ec, std::size_t) {
		write_timer->cancel();
			
		  if (ec) {
			// write failed—drop this connection entirely
			ps->stream.lowest_layer().close();
			return done(ec, "");
		  }
		  
		  read_timer->expires_after(std::chrono::seconds(5));
		  read_timer->async_wait([ps](auto ec){
			if (!ec) {
			  // cancel the read if it’s still pending
			  ps->stream.lowest_layer().cancel();
			}
		  });

		  http::async_read(ps->stream, *buffer, *res,
			[this, ps, buffer, res, read_timer, server, port, done]
			(boost::system::error_code ec, std::size_t) {
				read_timer->cancel();
				
			  if (ec) {
				// read failed or timed out—drop it
				ps->stream.lowest_layer().close();
				return done(ec, "");
			  }
			  
			  pool_.release(server, port, ps);

				auto status = res->result();
				if (status == http::status::unauthorized) {
				  cookies.clear();  // clear out any bad cookies
				  return done({},
							  "Unauthorized");
				}
				if (status != http::status::ok) {
				  LogWrite(PEERING__ERROR, 0, "Peering",
						   "%s: HTTP error %u", __FUNCTION__, status);
				  return done(
					boost::system::error_code(
					  static_cast<int>(status),
					  boost::asio::error::get_ssl_category()
					),
					"");
				}
			  // cookie logic
			  if (res->base().count(http::field::set_cookie)) {
				auto hdr = res->base()[http::field::set_cookie]
							   .to_string();
				std::istringstream ss(hdr);
				std::string token;
				while (std::getline(ss, token, ';')) {
				  auto pos = token.find('=');
				  if (pos!=std::string::npos) {
					cookies[token.substr(0,pos)] =
					  token.substr(pos+1);
				  }
				}
			  }
			  if (res->body() == "Unauthorized")
				cookies.clear();

			  done({}, res->body());
			});
		});
	});
}
