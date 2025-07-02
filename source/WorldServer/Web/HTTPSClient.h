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
#include <deque>
#include <boost/asio/executor_work_guard.hpp>

namespace ssl = boost::asio::ssl;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

// at the top of your .cpp:
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>            // for SSL_set_tlsext_host_name

// …

struct PooledStream {
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream;
  boost::asio::ip::tcp::resolver                         resolver;
  boost::asio::steady_timer                               connect_timer;
  boost::asio::steady_timer                               handshake_timer;

  PooledStream(boost::asio::io_context& ioc,
			   boost::asio::ssl::context& ssl_ctx)
	: stream(ioc, ssl_ctx)
	, resolver(ioc)
	, connect_timer(ioc)
	, handshake_timer(ioc)
  {}

  void prepare(const std::string& server,
			   const std::string& port,
			   std::function<void(boost::system::error_code)> on_ready)
  {
	auto endpoints = resolver.resolve(server, port);

	// — Step 1: async connect + 2s timeout
	connect_timer.expires_after(std::chrono::seconds(2));
	connect_timer.async_wait([this, on_ready](auto ec){
	  if (!ec) {
		stream.lowest_layer().cancel();
		on_ready(boost::asio::error::timed_out);
	  }
	});

	boost::asio::async_connect(
	  stream.lowest_layer(), endpoints,
	  [this, server, on_ready](auto ec, auto){
		connect_timer.cancel();
		if (ec) return on_ready(ec);

		// ** SNI: must set the host name for TLS before handshake **
		if (!SSL_set_tlsext_host_name(stream.native_handle(), server.c_str())) {
		  // pull the OpenSSL error and report it
		  auto err = ::ERR_get_error();
		  return on_ready(
			boost::system::error_code(
			  static_cast<int>(err),
			  boost::asio::error::get_ssl_category()
			)
		  );
		}

		// — Step 2: async handshake + 2s timeout
		handshake_timer.expires_after(std::chrono::seconds(2));
		handshake_timer.async_wait([this](auto ec){
		  if (!ec) stream.lowest_layer().cancel();
		});

		stream.async_handshake(
		  boost::asio::ssl::stream_base::client,
		  [this, on_ready](auto ec){
			handshake_timer.cancel();
			on_ready(ec);
		  }
		);
	  }
	);
  }
};

// --- ConnectionPool.h ---------------------------------------
class ConnectionPool {
public:
  ConnectionPool(boost::asio::io_context& ioc,
				 boost::asio::ssl::context& ssl_ctx)
	: ioc_(ioc), ssl_ctx_(ssl_ctx) {}

  // Acquire a ready PooledStream (connected + handshaken),
  // or create one and run prepare().
  void acquire(const std::string& server,
			   const std::string& port,
			   std::function<void(std::shared_ptr<PooledStream>, boost::system::error_code)> cb)
  {
	std::string key = server + ":" + port;
	{
	  std::lock_guard lk(mutex_);
	  auto &dq = free_[key];
	  if (!dq.empty()) {
		auto ps = dq.front();
		dq.pop_front();
		return cb(ps, {});
	  }
	}

	// no free stream → make a new one and prepare it
	auto ps = std::make_shared<PooledStream>(ioc_, ssl_ctx_);
	ps->prepare(server, port, [this, server, port, ps, cb](auto ec) {
	  if (ec) {
		cb(nullptr, ec);
	  } else {
		cb(ps, {});
	  }
	});
  }

  // Return a stream to the free list
  void release(const std::string& server,
			   const std::string& port,
			   std::shared_ptr<PooledStream> ps)
  {
	std::string key = server + ":" + port;
	// clear any leftover data in the buffer
	// (you might want to reset ps->buffer here if it’s stored inside)
	std::lock_guard lk(mutex_);
	free_[key].push_back(ps);
  }

private:
	boost::asio::io_context&                              ioc_;
	boost::asio::ssl::context&                            ssl_ctx_;
	std::mutex                                            mutex_;
	std::unordered_map<std::string, std::deque<std::shared_ptr<PooledStream>>> free_;
};

class HTTPSClient {
public:
	HTTPSClient(const std::string& certFile, const std::string& keyFile);
	~HTTPSClient();
	
	// — Async overloads —  
	void sendRequest(
		const std::string& server,
		const std::string& port,
		const std::string& target,
		std::function<void(boost::system::error_code, std::string)> done);

	void sendPostRequest(
		const std::string& server,
		const std::string& port,
		const std::string& target,
		const std::string& jsonPayload,
		std::function<void(boost::system::error_code, std::string)> done);

	// — Legacy synchronous wrappers —  
	std::string sendRequest(
		const std::string& server,
		const std::string& port,
		const std::string& target);

	std::string sendPostRequest(
		const std::string& server,
		const std::string& port,
		const std::string& target,
		const std::string& jsonPayload);

	std::string getServer() const { return server; }
	std::string getPort() const { return port; }

private:
	std::unordered_map<std::string, std::string> cookies;
    std::shared_ptr<boost::asio::ssl::context> sslCtx;
	boost::asio::io_context ioc_;
	 boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard_;
	ConnectionPool pool_;
	std::thread runner_;

	std::string certFile;
	std::string keyFile;
	std::string server;
	std::string port;

	void parseAndStoreCookies(const http::response<http::string_body>& res);
	std::shared_ptr<boost::asio::ssl::context> createSSLContext();  // New helper function
	std::string buildCookieHeader() const;
};

#endif // HTTPSCLIENT_H
