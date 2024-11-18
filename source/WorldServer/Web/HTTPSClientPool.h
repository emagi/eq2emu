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

#ifndef HTTPSCLIENTPOOL_H
#define HTTPSCLIENTPOOL_H

#include "HTTPSClient.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <boost/property_tree/ptree.hpp>  // Include Boost property tree

struct pair_hash {
	template <class T1, class T2>
	std::size_t operator()(const std::pair<T1, T2>& pair) const {
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};

class HTTPSClientPool {
public:
	HTTPSClientPool();
	~HTTPSClientPool();

	// init cert and key file
	void init(const std::string& cert, const std::string& key);

	// Pre-authenticate and add a client to the pool
	void addPeerClient(const std::string& peerId, const std::string& server, const std::string& port, const std::string& authEndpoint);

	std::shared_ptr<HTTPSClient> getOrCreateClient(const std::string& id, const std::string& server, const std::string& port);

	// Send a request to a peer by ID and parse response as a ptree
	boost::property_tree::ptree sendRequestToPeer(const std::string& peerId, const std::string& target);
	boost::property_tree::ptree sendPostRequestToPeer(const std::string& peerId, const std::string& target, const std::string& jsonPayload);
	void pollPeerHealthData(auto client, const std::string& id, const std::string& server, const std::string& port);

	void startPolling();       // Starts asynchronous polling of peers
	void stopPolling();        // Stops the polling process

	// Sends a POST request asynchronously by adding it to the task queue
	void sendPostRequestToPeerAsync(const std::string& peerId, const std::string& server, const std::string& port, const std::string& target, const std::string& payload);

	// Worker thread function
	void workerFunction();

	bool isPolling() { return running; }
private:
	std::shared_ptr<HTTPSClient> getClient(const std::string& peerId);

	std::unordered_map<std::pair<std::string, std::string>, std::shared_ptr<HTTPSClient>, pair_hash> clients;
	std::unordered_map<std::string, std::shared_ptr<HTTPSClient>> clientsById;  // New map for ID-based lookup
	std::string certFile;
	std::string keyFile;
	int pollingInterval;       // Polling interval in milliseconds

	std::queue<std::function<void()>> taskQueue;  // Queue of tasks to execute
	std::mutex queueMutex;                        // Mutex to protect access to the task queue
	std::condition_variable condition;            // Condition variable to signal worker threads
	std::vector<std::thread> workers;             // Worker threads
	bool stop = false;                            // Flag to stop workers

	std::atomic<bool> running; // Flag to control polling loop
	void pollPeerHealth(const std::string& server, const std::string& port); // Polls individual peer
};

#endif // HTTPSCLIENTPOOL_H
