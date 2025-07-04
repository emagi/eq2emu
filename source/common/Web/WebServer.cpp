/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2026  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

    This file is part of EQ2Emulator.

    EQ2Emulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EQ2Emulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "WebServer.h"
#include <iostream>
#include <sstream>
#include <random>
#include <chrono>
#include <boost/beast/core/detail/base64.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <string>
#include <iostream>

#include "../version.h"

#ifdef WORLD
	#include "../../WorldServer/WorldDatabase.h"
	extern WorldDatabase database;
#endif
#ifdef LOGIN
	#include "../../LoginServer/LoginDatabase.h"
	extern LoginDatabase database;
#endif

#ifdef WIN32
	#include <process.h>
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
	#include <conio.h>
#else
	#include <pthread.h>
	#include "../unix.h"
#endif

ThreadReturnType RunWebServer (void* tmp);

static std::string keypasswd = "";

void web_handle_version(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    res.set(http::field::content_type, "application/json");
	boost::property_tree::ptree pt;
	
    // Add key-value pairs to the property tree
    pt.put("eq2emu_process", std::string(EQ2EMU_MODULE));
    pt.put("version", std::string(CURRENT_VERSION));
    pt.put("compile_date", std::string(COMPILE_DATE));
    pt.put("compile_time", std::string(COMPILE_TIME));

    // Create an output string stream to hold the JSON string
    std::ostringstream oss;
    
    // Write the property tree to the output string stream as JSON
    boost::property_tree::write_json(oss, pt);

    // Get the JSON string from the output string stream
    std::string json = oss.str();
    res.body() = json;
    res.prepare_payload();
}

void web_handle_root(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    res.set(http::field::content_type, "text/html");
    res.body() = "Hello!";
    res.prepare_payload();
}

// this function is called to obtain password info about an encrypted key
std::string WebServer::my_password_callback(
    std::size_t max_length,  // the maximum length for a password
    ssl::context::password_purpose purpose ) // for_reading or for_writing
{
	return keypasswd;
}

//void handle_root(const http::request<http::string_body>& req, http::response<http::string_body>& res);

WebServer::WebServer(const std::string& address, unsigned short port, const std::string& cert_file, const std::string& key_file, const std::string& key_password, const std::string& hardcode_user, const std::string& hardcode_password)
	: ioc_(1),
	  ssl_ctx_(ssl::context::tlsv13_server),
	  acceptor_(ioc_, {boost_net::ip::make_address(address), port}) {
		  keypasswd = key_password;
	// Initialize SSL context
	if(cert_file.size() < 1 || key_file.size() < 1) {
		is_ssl = false;
	}
	else {
		ssl_ctx_.set_password_callback(my_password_callback);
		ssl_ctx_.use_certificate_chain_file(cert_file);
		ssl_ctx_.use_private_key_file(key_file, ssl::context::file_format::pem);
		is_ssl = true;
	}
	keypasswd = ""; // reset no longer needed
	
	if(hardcode_user.size() > 0 && hardcode_password.size() > 0)
		credentials_[hardcode_user] = hardcode_password;
	
	register_route("/", web_handle_root);
	register_route("/version", web_handle_version);
}

WebServer::~WebServer() {
	ioc_.stop();
}

ThreadReturnType RunWebServer (void* tmp) {
	if(tmp == nullptr) {
		THREAD_RETURN(NULL);
	}
	WebServer* ws = (WebServer*)tmp;
    ws->start();
	THREAD_RETURN(NULL);
}

void WebServer::start() {
	do_accept();
    ioc_.run();
}

void WebServer::run() {
	pthread_t thread;
	pthread_create(&thread, NULL, RunWebServer, this);
	pthread_detach(thread);
}


void WebServer::register_route(const std::string& uri, std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&)> handler, bool auth_req) {
	int32 status = database.NoAuthRoute((char*)uri.c_str()); // overrides the default hardcode settings via DB
	if(status == 0) {
			auth_req = false;
	}
	if(auth_req) {
		routes_[uri] = handler;
	}
	else {
		noauth_routes_[uri] = handler;
	}
	route_required_status_[uri] = status;
}

void WebServer::do_accept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            this->on_accept(ec, std::move(socket));
        });
}

void WebServer::on_accept(beast::error_code ec, tcp::socket socket) {
    if (!ec) {
		if(is_ssl) {
			std::thread(&WebServer::do_session_ssl, this, std::move(socket)).detach();
		}
		else {
			std::thread(&WebServer::do_session, this, std::move(socket)).detach();
		}
    }
    do_accept();
}

void WebServer::do_session(tcp::socket socket) {
    try {
        bool close = false;
        beast::flat_buffer buffer;

        while (!close) {
            // 1) Read a complete request
            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            // 2) Invoke your handler, giving it a lambda that
            //    sets up version/keep_alive on the response
            handle_request(std::move(req), [&](auto&& response) {
                // mirror HTTP version
                response.version(req.version());
                // propagate the client’s keep-alive choice
                response.keep_alive(req.keep_alive());

                // if the client asked us to close, mark for shutdown
                if (! req.keep_alive())
                    close = true;

                http::write(socket, response);
            });

            // 3) Discard anything left in the buffer so the next
            //    http::read starts fresh
            buffer.consume(buffer.size());
        }

        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_send, ec);
    }
    catch (const std::exception& e) {
		// irrelevant spam for now really
    }
}

void WebServer::do_session_ssl(tcp::socket socket) {
    try {
        ssl::stream<tcp::socket> stream(std::move(socket), ssl_ctx_);
        stream.handshake(ssl::stream_base::server);

        bool close = false;
        beast::flat_buffer buffer;

        while (!close) {
            http::request<http::string_body> req;
            http::read(stream, buffer, req);

            handle_request(std::move(req), [&](auto&& response) {
                response.version(req.version());
                response.keep_alive(req.keep_alive());
                if (! req.keep_alive())
                    close = true;
                http::write(stream, response);
            });

            buffer.consume(buffer.size());
        }

        beast::error_code ec;
        stream.next_layer().shutdown(tcp::socket::shutdown_send, ec);
    }
    catch (const std::exception& e) {
		// irrelevant spam for now really
    }
}

template <class Body, class Allocator>
void WebServer::handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, std::function<void(http::response<http::string_body>&&)> send) {
    auto it = noauth_routes_.find(req.target().to_string());
    if (it != noauth_routes_.end()) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        it->second(req, res);
        return send(std::move(res));
    }
	int32 user_status = 0;
	std::string session_id = authenticate(req, &user_status);
    if (session_id.size() < 1) {
        http::response<http::string_body> res{http::status::unauthorized, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::www_authenticate, "Basic realm=\"example\"");
        res.body() = "Unauthorized";
        res.prepare_payload();
        return send(std::move(res));
    }
	
	auto status_it = route_required_status_.find(req.target().to_string());
    if (status_it != route_required_status_.end()) {
		if(status_it->second > 0 && status_it->second != 0xFFFFFFFF && status_it->second > user_status) {
			http::response<http::string_body> res{http::status::unauthorized, req.version()};
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.body() = "Unauthorized status";
			res.prepare_payload();
			return send(std::move(res));
		}
	}

    it = routes_.find(req.target().to_string());
    if (it != routes_.end()) {
        http::response<http::string_body> res{http::status::ok, req.version()};
		res.set(http::field::set_cookie, "session_id=" + session_id);
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        it->second(req, res);
        return send(std::move(res));
    }
/*

    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.body() = "Not Found";
    res.prepare_payload();
    return send(std::move(res));
	*/
    return send(http::response<http::string_body>{http::status::bad_request, req.version()});
}


std::string WebServer::authenticate(const http::request<http::string_body>& req, int32* user_status) {
    auto it = req.find(http::field::cookie);
    if (it != req.end()) {
        std::istringstream cookie_stream(it->value().to_string());
        std::string session_id;
        std::getline(cookie_stream, session_id, '=');
        if (session_id == "session_id") {
            std::string id;
            std::getline(cookie_stream, id);
            if (sessions_.find(id) != sessions_.end()) {
				if(sessions_status_.find(id) != sessions_status_.end()) {
					*user_status = sessions_status_[id];
				}
                return id;
            }
        }
    }

    it = req.find(http::field::authorization);
    if (it != req.end()) {
        std::string auth_header = it->value().to_string();
        if (auth_header.substr(0, 6) == "Basic ") {
            std::string encoded_credentials = auth_header.substr(6);
            std::string decoded_credentials;
            decoded_credentials.resize(boost::beast::detail::base64::decoded_size(encoded_credentials.size()));
            auto result = boost::beast::detail::base64::decode(
                &decoded_credentials[0], 
                encoded_credentials.data(), 
                encoded_credentials.size()
            );
            decoded_credentials.resize(result.first);

            std::istringstream credentials_stream(decoded_credentials);
            std::string username, password;
            std::getline(credentials_stream, username, ':');
            std::getline(credentials_stream, password);
			int32 out_status = 0;
            if ((credentials_.find(username) != credentials_.end() && credentials_[username] == password) || (database.AuthenticateWebUser((char*)username.c_str(),(char*)password.c_str(), &out_status) > 0)) {
                std::string session_id = generate_session_id();
                sessions_[session_id] = username;
				sessions_status_[session_id] = out_status;
				*user_status = out_status;
                return session_id;
            }
        }
    }

    return std::string("");
}

std::string WebServer::generate_session_id() {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<> dist(0, 15);
    std::string session_id;
    for (int i = 0; i < 32; ++i) {
        session_id += "0123456789abcdef"[dist(rng)];
    }
    return session_id;
}

// Explicit template instantiation
template void WebServer::handle_request<http::string_body, std::allocator<char>>(
    http::request<http::string_body, http::basic_fields<std::allocator<char>>>&&, 
    std::function<void(http::response<http::string_body>&&)>
);