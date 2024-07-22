#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <string>
#include <thread>
#include <functional>
#include <unordered_map>

#include "../types.h"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace boost_net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;   // from <boost/asio/ssl.hpp>
using tcp = boost_net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

class WebServer {
public:
    WebServer(const std::string& address, unsigned short port, const std::string& cert_file, const std::string& key_file, const std::string& key_password, const std::string& hardcode_user, const std::string& hardcode_password);
	~WebServer();
	
    void run();
	void start();
	
	void register_route(const std::string& uri, std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&)> handler, bool auth_required = true);
private:
	bool is_ssl;
	static std::string my_password_callback(std::size_t max_length, ssl::context::password_purpose purpose);
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
    void do_session_ssl(tcp::socket socket);
    void do_session(tcp::socket socket);
	
    template <class Body, class Allocator>
    void handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, std::function<void(http::response<http::string_body>&&)> send);

    std::string authenticate(const http::request<http::string_body>& req, int32* user_status = 0);
    std::string generate_session_id();

    boost_net::io_context ioc_;
    ssl::context ssl_ctx_;
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, std::string> sessions_;  // session_id -> username
    std::unordered_map<std::string, int32> sessions_status_;  // session_id -> status
	
    std::unordered_map<std::string, std::string> credentials_; // username -> password
    std::unordered_map<std::string, int32> route_required_status_; // route -> status
    std::unordered_map<std::string, std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&)>> routes_;
    std::unordered_map<std::string, std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&)>> noauth_routes_;
};
