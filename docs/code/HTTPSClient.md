# File: `HTTPSClient.h`

## Classes

- `HTTPSClient`

## Functions

- `std::string sendRequest(const std::string& server, const std::string& port, const std::string& target);`
- `std::string sendPostRequest(const std::string& server, const std::string& port, const std::string& target, const std::string& jsonPayload);`
- `std::string getServer() const { return server; }`
- `std::string getPort() const { return port; }`
- `void parseAndStoreCookies(const http::response<http::string_body>& res);`
- `std::shared_ptr<boost::asio::ssl::context> createSSLContext();  // New helper function`
- `std::string buildCookieHeader() const;`

## Notable Comments

- /*
- */
- // Send a request with stored cookies and return response as string
- // Send a POST request with a JSON payload and return response as string
