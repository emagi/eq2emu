# File: `HTTPSClientPool.h`

## Classes

- `pair_hash`
- `HTTPSClientPool`

## Functions

- `std::size_t operator()(const std::pair<T1, T2>& pair) const {`
- `return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);`
- `void init(const std::string& cert, const std::string& key);`
- `void addPeerClient(const std::string& peerId, const std::string& server, const std::string& port, const std::string& authEndpoint);`
- `std::shared_ptr<HTTPSClient> getOrCreateClient(const std::string& id, const std::string& server, const std::string& port);`
- `boost::property_tree::ptree sendRequestToPeer(const std::string& peerId, const std::string& target);`
- `boost::property_tree::ptree sendPostRequestToPeer(const std::string& peerId, const std::string& target, const std::string& jsonPayload);`
- `void pollPeerHealthData(std::shared_ptr<HTTPSClient> client, const std::string& id, const std::string& server, const std::string& port);`
- `void startPolling();       // Starts asynchronous polling of peers`
- `void stopPolling();        // Stops the polling process`
- `void sendPostRequestToPeerAsync(const std::string& peerId, const std::string& server, const std::string& port, const std::string& target, const std::string& payload);`
- `void workerFunction();`
- `bool isPolling() { return running; }`
- `std::shared_ptr<HTTPSClient> getClient(const std::string& peerId);`
- `void pollPeerHealth(const std::string& server, const std::string& port); // Polls individual peer`

## Notable Comments

- /*
- */
- // init cert and key file
- // Pre-authenticate and add a client to the pool
- // Send a request to a peer by ID and parse response as a ptree
- // Sends a POST request asynchronously by adding it to the task queue
- // Worker thread function
