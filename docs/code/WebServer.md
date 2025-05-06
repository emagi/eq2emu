# File: `WebServer.h`

## Classes

- `WebServer`

## Functions

- `void run();`
- `void start();`
- `void register_route(const std::string& uri, std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&)> handler, bool auth_required = true);`
- `void do_accept();`
- `void on_accept(beast::error_code ec, tcp::socket socket);`
- `void do_session_ssl(tcp::socket socket);`
- `void do_session(tcp::socket socket);`
- `void handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, std::function<void(http::response<http::string_body>&&)> send);`
- `std::string authenticate(const http::request<http::string_body>& req, int32* user_status = 0);`
- `std::string generate_session_id();`

## Notable Comments

_None detected_
