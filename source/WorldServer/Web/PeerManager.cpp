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

#include "PeerManager.h"
#include "../../common/Log.h"
#include "../net.h"
#include "../PlayerGroups.h"
#include "HTTPSClientPool.h"
#include "../Rules/Rules.h"

extern NetConnection net;
extern HTTPSClientPool peer_https_pool;
extern RuleManager rule_manager;

// HealthCheck method definitions
void HealthCheck::updateStatus(HealthStatus newStatus) {
	status = newStatus;
	lastReceived = std::chrono::system_clock::now();
}

std::chrono::duration<double> HealthCheck::timeSinceLastCheck() const {
	return std::chrono::system_clock::now() - lastReceived;
}

ZoneChangeDetails::ZoneChangeDetails(std::string peer_id, std::string peer_world_address, std::string peer_internal_world_address, int16 peer_world_port,
	std::string peer_web_address, int16 peer_web_port, std::string zone_file_name, std::string zone_name, int32 zone_id,
	int32 instance_id, float safe_x, float safe_y, float safe_z, float safe_heading, bool lock_state, sint16 min_status,
	int16 min_level, int16 max_level, int16 min_version, int32 default_lockout_time, int32 default_reenter_time, int8 instance_type, int32 num_players)
	: peerId(std::move(peer_id)), peerWorldAddress(peer_world_address), peerInternalWorldAddress(peer_internal_world_address), peerWorldPort(peer_world_port),
	peerWebAddress(peer_web_address), peerWebPort(peer_web_port), zoneFileName(zone_file_name), zoneName(zone_name), zoneId(zone_id), instanceId(instance_id),
	safeX(safe_x), safeY(safe_y), safeZ(safe_z), safeHeading(safe_heading), lockState(lock_state), minStatus(min_status), minLevel(min_level),
	maxLevel(max_level), minVersion(min_version), defaultLockoutTime(default_lockout_time), defaultReenterTime(default_reenter_time), instanceType(instance_type), numPlayers(num_players) {
	zonePtr = nullptr;
}

ZoneChangeDetails::ZoneChangeDetails(ZoneChangeDetails* copy_details) : peerId(copy_details->peerId), peerWorldAddress(copy_details->peerWorldAddress), peerWorldPort(copy_details->peerWorldPort),
peerWebAddress(copy_details->peerWebAddress), peerWebPort(copy_details->peerWebPort), zoneFileName(copy_details->zoneFileName), zoneName(copy_details->zoneName),
zoneId(copy_details->zoneId), instanceId(copy_details->instanceId), safeX(copy_details->safeX), safeY(copy_details->safeY), safeZ(copy_details->safeZ),
safeHeading(copy_details->safeHeading), lockState(copy_details->lockState), minStatus(copy_details->minStatus), minLevel(copy_details->minLevel),
maxLevel(copy_details->maxLevel), minVersion(copy_details->minVersion), defaultLockoutTime(copy_details->defaultLockoutTime),
defaultReenterTime(copy_details->defaultReenterTime), instanceType(copy_details->instanceType), numPlayers(copy_details->numPlayers),
peerAuthorized(copy_details->peerAuthorized), zoneKey(copy_details->zoneKey), authDispatchedTime(copy_details->authDispatchedTime),
zoningPastAuth(copy_details->zoningPastAuth), zonePtr(copy_details->zonePtr) {

}

// PeerManager method definitions
void PeerManager::addPeer(std::string id, PeeringStatus status, std::string client_address, std::string client_internal_address, int16 client_port, std::string web_address, int16 web_port) {
	std::shared_ptr<Peer> peer = std::make_shared<Peer>(id, PeeringStatus::SECONDARY, client_address, client_internal_address, client_port, web_address, web_port);
	peers.emplace(id, peer);
}

void PeerManager::updateHealth(const std::string& id, HealthStatus newStatus) {
	if (peers.find(id) != peers.end()) {
		peers[id]->healthCheck.updateStatus(newStatus);
	}
}

void PeerManager::updatePriority(const std::string& id, int16 priority) {
	if (peers.find(id) != peers.end()) {
		peers[id]->peerPriority = priority;
	}
}

void PeerManager::updateZoneTree(const std::string& id, const boost::property_tree::ptree& newTree) {
	auto it = peers.find(id);
	if (it != peers.end()) {
		std::lock_guard<std::mutex> lock(it->second->dataMutex);
		*(it->second->zone_tree) = newTree;
	}
}

void PeerManager::updateClientTree(const std::string& id, const boost::property_tree::ptree& newTree) {
	auto it = peers.find(id);
	if (it != peers.end()) {
		std::lock_guard<std::mutex> lock(it->second->dataMutex);
		*(it->second->client_tree) = newTree;
	}
}

void PeerManager::setZonePeerData(ZoneChangeDetails* opt_details, std::string peerId, std::string peerWorldAddress, std::string peerInternalWorldAddress, int16 peerWorldPort,
	std::string peerWebAddress, int16 peerWebPort, std::string zoneFileName,
	std::string zoneName, int32 zoneId, int32 instanceId, float safeX, float safeY, float safeZ, float safeHeading, bool lockState, sint16 minStatus,
	int16 minLevel, int16 maxLevel, int16 minVersion, int32 defaultLockoutTime, int32 defaultReenterTime, int8 instanceType, int32 numPlayers, bool isCityZone) {
	if (opt_details) {
		opt_details->peerId = peerId;
		opt_details->peerWorldAddress = peerWorldAddress;
		opt_details->peerInternalWorldAddress = peerInternalWorldAddress;
		opt_details->peerWorldPort = peerWorldPort;
		opt_details->peerWebAddress = peerWebAddress;
		opt_details->peerWebPort = peerWebPort;
		opt_details->zoneFileName = zoneFileName;
		opt_details->zoneName = zoneName;
		opt_details->zoneId = zoneId;
		opt_details->instanceId = instanceId;
		opt_details->safeX = safeX;
		opt_details->safeY = safeY;
		opt_details->safeZ = safeZ;
		opt_details->safeHeading = safeHeading;
		opt_details->lockState = lockState;
		opt_details->minStatus = minStatus;
		opt_details->minLevel = minLevel;
		opt_details->maxLevel = maxLevel;
		opt_details->minVersion = minVersion;
		opt_details->minVersion = minVersion;
		opt_details->defaultLockoutTime = defaultLockoutTime;
		opt_details->defaultReenterTime = defaultReenterTime;
		opt_details->instanceType = instanceType;
		opt_details->numPlayers = numPlayers;
		opt_details->isCityZone = isCityZone;
		opt_details->zonePtr = nullptr;
		opt_details->peerAuthorized = false;
		opt_details->zoneKey = 0;
		opt_details->authDispatchedTime = 0;
		opt_details->zoningPastAuth = false;
	}
}

void PeerManager::setZonePeerDataSelf(ZoneChangeDetails* opt_details, std::string zoneFileName, std::string zoneName, int32 zoneId,
	int32 instanceId, float safeX, float safeY, float safeZ, float safeHeading, bool lockState, sint16 minStatus,
	int16 minLevel, int16 maxLevel, int16 minVersion, int32 defaultLockoutTime, int32 defaultReenterTime, int8 instanceType,
	int32 numPlayers, bool isCityZone, void* zonePtr) {
	if (opt_details) {
		opt_details->peerId = "self";
		opt_details->peerWorldAddress = net.GetWorldAddress();
		opt_details->peerInternalWorldAddress = net.GetInternalWorldAddress();
		opt_details->peerWorldPort = net.GetWorldPort();
		opt_details->peerWebAddress = net.GetWebWorldAddress();
		opt_details->peerWebPort = net.GetWebWorldPort();
		opt_details->zoneFileName = zoneFileName;
		opt_details->zoneName = zoneName;
		opt_details->zoneId = zoneId;
		opt_details->instanceId = instanceId;
		opt_details->safeX = safeX;
		opt_details->safeY = safeY;
		opt_details->safeZ = safeZ;
		opt_details->safeHeading = safeHeading;
		opt_details->lockState = lockState;
		opt_details->minStatus = minStatus;
		opt_details->minLevel = minLevel;
		opt_details->maxLevel = maxLevel;
		opt_details->minVersion = minVersion;
		opt_details->defaultLockoutTime = defaultLockoutTime;
		opt_details->defaultReenterTime = defaultReenterTime;
		opt_details->instanceType = instanceType;
		opt_details->numPlayers = numPlayers;
		opt_details->isCityZone = isCityZone;
		opt_details->zonePtr = zonePtr;
		opt_details->peerAuthorized = true;
		opt_details->zoneKey = 0;
		opt_details->authDispatchedTime = 0;
		opt_details->zoningPastAuth = true;
	}
}

std::string PeerManager::getZonePeerId(const std::string& inc_zone_name, int32 inc_zone_id, int32 inc_instance_id, ZoneChangeDetails* opt_details, bool only_always_loaded, int32 matchDuplicatedId) {
	bool matchFullZone = false;
	std::string fullZoneId = "";
	for (auto& [peerId, peer] : peers) {
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;
		try {
			std::lock_guard<std::mutex> lock(peer->dataMutex);
			for (const auto& zone : peer->zone_tree->get_child("Zones")) {
				// Access each field within the current zone
				std::string zone_name = zone.second.get<std::string>("zone_name");
				std::string zone_file_name = zone.second.get<std::string>("zone_file_name");
				int32 zone_id = zone.second.get<int32>("zone_id");
				int32 instance_id = zone.second.get<int32>("instance_id");
				bool shutting_down = zone.second.get<std::string>("shutting_down") == "true";
				bool instance_zone = zone.second.get<std::string>("instance_zone") == "true";
				int32 num_players = zone.second.get<int32>("num_players");
				bool city_zone = zone.second.get<std::string>("city_zone") == "true";
				float safe_x = zone.second.get<float>("safe_x");
				float safe_y = zone.second.get<float>("safe_y");
				float safe_z = zone.second.get<float>("safe_z");
				float safe_heading = zone.second.get<float>("safe_heading");
				bool lock_state = zone.second.get<bool>("lock_state");
				sint16 min_status = zone.second.get<sint16>("min_status");
				int16 min_level = zone.second.get<int16>("min_level");
				int16 max_level = zone.second.get<int16>("max_level");
				int16 min_version = zone.second.get<int16>("min_version");
				int32 default_lockout_time = zone.second.get<int32>("default_lockout_time");
				int32 default_reenter_time = zone.second.get<int32>("default_reenter_time");
				int8 instance_type = zone.second.get<int8>("instance_type");
				bool always_loaded = zone.second.get<bool>("always_loaded");
				int32 duplicate_id = zone.second.get<int32>("duplicated_id");

				if (only_always_loaded && !always_loaded)
					continue;

				if (!shutting_down) {
					bool match = false;
					if(matchDuplicatedId > 0 && duplicate_id != matchDuplicatedId)
						continue;
					
					if (instance_zone && inc_instance_id > 0 && instance_id == inc_instance_id) {
						match = true;
					}
					else if (!instance_zone && inc_instance_id == 0 && inc_zone_id > 0 && zone_id == inc_zone_id) {
						match = true;
					}
					else if (!instance_zone && inc_zone_name.length() > 0 && strncasecmp(zone_name.c_str(), inc_zone_name.c_str(), inc_zone_name.length()) == 0) {
						match = true;
					}
						
					int32 max_players = rule_manager.GetZoneRule(zone_id, R_Zone, SharedZoneMaxPlayers)->GetInt32();
					if(max_players < 1) // default of 30
						max_players = 30;
					if(!instance_zone && num_players >= max_players && !city_zone) {
						match = false;
						setZonePeerData(opt_details, peerId, peer->worldAddr, peer->internalWorldAddr, peer->worldPort, peer->webAddr, peer->webPort, zone_file_name, zone_name, zone_id, instance_id,
							safe_x, safe_y, safe_z, safe_heading, lock_state, min_status, min_level, max_level, min_version, default_lockout_time, default_reenter_time, instance_type, num_players, city_zone);
						matchFullZone = true;
						fullZoneId = peerId;
					}

					if (match) {
						setZonePeerData(opt_details, peerId, peer->worldAddr, peer->internalWorldAddr, peer->worldPort, peer->webAddr, peer->webPort, zone_file_name, zone_name, zone_id, instance_id,
							safe_x, safe_y, safe_z, safe_heading, lock_state, min_status, min_level, max_level, min_version, default_lockout_time, default_reenter_time, instance_type, num_players, city_zone);
						return peerId;
					}
				}
			}
		}
		catch (const std::exception& e) {
			LogWrite(PEERING__ERROR, 0, "Peering", "%s: Error Parsing Zones for %s:%u", __FUNCTION__, peer->webAddr.c_str(), peer->webPort);
		}
	}
	
	return fullZoneId;
}

int32 PeerManager::getZoneHighestDuplicateId(const std::string& inc_zone_name, int32 inc_zone_id, bool increment_new_value) {
	int32 highestID = 0;
	bool matched_zone = false;
	for (auto& [peerId, peer] : peers) {
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;
		try {
			std::lock_guard<std::mutex> lock(peer->dataMutex);
			for (const auto& zone : peer->zone_tree->get_child("Zones")) {
				// Access each field within the current zone
				std::string zone_name = zone.second.get<std::string>("zone_name");
				bool instance_zone = zone.second.get<std::string>("instance_zone") == "true";
				std::string zone_file_name = zone.second.get<std::string>("zone_file_name");
				int32 zone_id = zone.second.get<int32>("zone_id");
				int32 instance_id = zone.second.get<int32>("instance_id");
				int32 duplicate_id = zone.second.get<int32>("duplicated_id");

				bool match = false;
				if (!instance_zone && inc_zone_id > 0 && zone_id == inc_zone_id) {
					match = true;
				}
				else if (!instance_zone && inc_zone_name.length() > 0 && strncasecmp(zone_name.c_str(), inc_zone_name.c_str(), inc_zone_name.length()) == 0) {
					match = true;
				}
					
				if (match) {
					matched_zone = true;
					if(duplicate_id > highestID)
						highestID = duplicate_id;
				}
			}
		}
		catch (const std::exception& e) {
			LogWrite(PEERING__ERROR, 0, "Peering", "%s: Error Parsing Zones for %s:%u", __FUNCTION__, peer->webAddr.c_str(), peer->webPort);
		}
	}
	
	if(matched_zone && increment_new_value) {
		highestID++;
	}
	
	return highestID;
}

void PeerManager::handlePrimaryConflict(const std::string& reconnectingPeerId) {
	// Compare IDs or priorities to decide on the primary role
	auto currentPrimary = getCurrentPrimary();
	auto reconnectingPeer = getPeerById(reconnectingPeerId);

	if (currentPrimary && (currentPrimary->peerPriority > reconnectingPeer->peerPriority || currentPrimary->healthCheck.status != HealthStatus::OK)) {
		// Demote the current primary
		if (reconnectingPeer && currentPrimary->healthCheck.status == HealthStatus::OK) {
			setPrimary(reconnectingPeerId);
			LogWrite(PEERING__INFO, 0, "Peering", "%s: Peer %s forced to primary", __FUNCTION__, reconnectingPeer->id.c_str());
			if (currentPrimary) {
				LogWrite(PEERING__INFO, 0, "Peering", "%s: Demoted to secondary", __FUNCTION__);
			}
		}
	}
	else {
		// Demote the reconnecting peer
		if (currentPrimary && currentPrimary->healthCheck.status == HealthStatus::OK) {
			setPrimary(currentPrimary->id);
			LogWrite(PEERING__INFO, 0, "Peering", "%s: Peer %s forced to primary", __FUNCTION__, currentPrimary->id.c_str());
		}
	}
}

void PeerManager::setPrimary(const std::string& id) {
	for (auto& [peerId, peer] : peers) {
		peer->peeringStatus = (peerId == id) ? PeeringStatus::PRIMARY : PeeringStatus::SECONDARY;
	}
}

bool PeerManager::hasPrimary() {
	for (auto& [peerId, peer] : peers) {
		if (peer->peeringStatus == PeeringStatus::PRIMARY)
			return true;
	}
	return false;
}

bool PeerManager::hasPriorityPeer(int16 priority) {
	for (auto& [peerId, peer] : peers) {
		if (peer->peerPriority == priority && peer->healthCheck.status == HealthStatus::OK)
			return true;
	}
	return false;
}

std::string PeerManager::getPriorityPeer() {
	int16 peerPriority = 65535;
	std::string id = "";
	for (auto& [peerId, peer] : peers) {
		if (peer->healthCheck.status > HealthStatus::ERROR && peer->healthCheck.status <= HealthStatus::OK &&
			peer->peerPriority > 0 && peer->peerPriority < peerPriority) {
			peerPriority = peer->peerPriority;
			id = peer->id;
		}
	}
	return id;
}

void PeerManager::updatePeer(const std::string& web_address, int16 web_port, const std::string& client_address, const std::string& client_internal_address, int16 client_port, bool is_primary) {
	for (auto& [peerId, peer] : peers) {
		if (peer->webAddr == web_address && peer->webPort == web_port) {
			peer->worldAddr = client_address;
			peer->worldPort = client_port;
			peer->internalWorldAddr = client_internal_address;
			if (is_primary) {
				peer->peeringStatus = PeeringStatus::PRIMARY;
			}
			else {
				peer->peeringStatus = PeeringStatus::SECONDARY;
			}
			break;
		}
	}
}

std::string PeerManager::isPeer(const std::string& web_address, int16 web_port) {
	for (auto& [peerId, peer] : peers) {
		if (peer->webAddr == web_address && peer->webPort == web_port) {
			return peerId;
		}
	}
	return std::string("");
}


HealthStatus PeerManager::getPeerStatus(const std::string& web_address, int16 web_port) {
	for (auto& [peerId, peer] : peers) {
		if (peer->webAddr == web_address && peer->webPort == web_port) {
			return peer->healthCheck.status;
		}
	}
	return HealthStatus::UNKNOWN;
}

bool PeerManager::hasPeers() {
	return (peers.size() > 0);
}

std::string PeerManager::assignUniqueNameForSecondary(const std::string& baseName, std::string client_address, std::string client_internal_address, int16 client_port, std::string web_address, int16 web_port) {
	int suffix = 1;
	std::string uniqueName = baseName + std::to_string(suffix);

	while (peers.find(uniqueName) != peers.end()) {
		uniqueName = baseName + std::to_string(suffix);
		++suffix;
	}

	addPeer(uniqueName, PeeringStatus::SECONDARY, client_address, client_internal_address, client_port, web_address, web_port);

	updateHealth(uniqueName, HealthStatus::STARTUP);

	return uniqueName;
}

std::optional<std::string> PeerManager::getHealthyPeer() const {
	for (const auto& [id, peer] : peers) {
		if (peer->healthCheck.status == HealthStatus::OK) {
			return id;
		}
	}
	return std::nullopt;
}

std::shared_ptr<Peer> PeerManager::getHealthyPeerPtr() const {
	for (const auto& [id, peer] : peers) {
		if (peer->healthCheck.status == HealthStatus::OK) {
			return peer;
		}
	}
	return nullptr;
}

std::shared_ptr<Peer> PeerManager::getHealthyPrimaryPeerPtr() const {
	for (const auto& [id, peer] : peers) {
		if (peer->peeringStatus == PeeringStatus::PRIMARY && peer->healthCheck.status == HealthStatus::OK) {
			return peer;
		}
	}
	return nullptr;
}

std::shared_ptr<Peer> PeerManager::getHealthyPeerWithLeastClients() const {
	std::vector<std::shared_ptr<Peer>> healthyPeers;

	// Seed random generator
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	// Step 1: Collect healthy peers
	for (auto& [peerId, peer] : peers) {

		// if setup to distribute to peers only, skip primary
		if (peer->peerPriority == 0 && peer->peeringStatus == PeeringStatus::PRIMARY) {
			continue;
		}

		if (peer->healthCheck.status == HealthStatus::OK) {
			healthyPeers.push_back(peer);
		}
	}

	if (healthyPeers.empty()) {
		return nullptr;  // No healthy peers found
	}

	std::string treeList("Zones");

	// Step 2: Determine minimum number of "Clients" for healthy peers
	size_t minClientCount = std::numeric_limits<size_t>::max();
	for (const auto& peer : healthyPeers) {
		std::lock_guard<std::mutex> lock(peer->dataMutex);  // Lock for thread-safe access
		std::shared_ptr<boost::property_tree::ptree> tree = peer->zone_tree;
		if (auto clientOpt = tree->get_child_optional(treeList.c_str())) {
			size_t clientCount = clientOpt.get().size();
			if (clientCount < minClientCount) {
				minClientCount = clientCount;
			}
		}
		else {
			// Consider peers without "Clients" node as having 0 clients
			minClientCount = 0;
		}
	}

	// Step 3: Collect all healthy peers with minClientCount
	std::vector<std::shared_ptr<Peer>> minClientPeers;
	for (const auto& peer : healthyPeers) {
		std::lock_guard<std::mutex> lock(peer->dataMutex);
		std::shared_ptr<boost::property_tree::ptree> tree = peer->zone_tree;
		size_t clientCount = 0;
		if (auto clientOpt = tree->get_child_optional(treeList.c_str())) {
			clientCount = clientOpt.get().size();
		}
		if (clientCount == minClientCount) {
			minClientPeers.push_back(peer);
		}
	}

	// Step 4: Select a random peer from the minClientPeers
	if (!minClientPeers.empty()) {
		size_t randomIndex = std::rand() % minClientPeers.size();
		return minClientPeers[randomIndex];
	}

	return nullptr; // Fallback if no peers match the criteria
}

std::shared_ptr<Peer> PeerManager::getPeerById(const std::string& id) const {
	auto it = peers.find(id);
	if (it != peers.end()) {
		return it->second;  // Return the shared_ptr<Peer> if found
	}
	return nullptr;  // Return nullptr if the peerId doesn't exist
}
// Function to get a unique integer for a peer
int32 PeerManager::getUniqueGroupId() {
	std::lock_guard<std::mutex> lock(idMutex);
	uniqueGroupID++;
	if (uniqueGroupID == 0)
		uniqueGroupID++;

	return uniqueGroupID;
}

bool PeerManager::sendPrimaryNewGroupRequest(std::string leader, std::string member, int32 entity_id, GroupOptions* options) {
	std::shared_ptr<Peer> primary = getHealthyPrimaryPeerPtr();
	if (primary) {
		boost::property_tree::ptree root;

		root.put("peer_web_address", std::string(net.GetWebWorldAddress()));
		root.put("peer_web_port", std::to_string(net.GetWebWorldPort()));
		root.put("group_id", 0);
		root.put("leader_name", leader);
		root.put("member_name", member);
		root.put("member_entity_id", entity_id);
		populateGroupOptions(root, options);
		root.put("is_update", false);

		std::ostringstream jsonStream;
		boost::property_tree::write_json(jsonStream, root);
		std::string jsonPayload = jsonStream.str();
		LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Leader %s, Member %s(%u)", __FUNCTION__, leader.c_str(), member.c_str(), entity_id);
		peer_https_pool.sendPostRequestToPeerAsync(primary->id, primary->webAddr, std::to_string(primary->webPort), "/newgroup", jsonPayload);
		return true;
	}

	return false;
}

// Helper function to populate the ptree with GroupOptions
void PeerManager::populateGroupOptions(boost::property_tree::ptree& root, GroupOptions* options) {
	if (!options) return; // Handle null pointer gracefully
	root.put("loot_method", options->loot_method);
	root.put("loot_items_rarity", options->loot_items_rarity);
	root.put("auto_split", options->auto_split);
	root.put("default_yell", options->default_yell);
	root.put("group_lock_method", options->group_lock_method);
	root.put("group_autolock", options->group_autolock);
	root.put("solo_autolock", options->solo_autolock);
	root.put("auto_loot_method", options->auto_loot_method);
	root.put("last_looted_index", options->last_looted_index);
}

void PeerManager::sendPeersNewGroupRequest(std::string peer_creation_address, int16 peer_creation_port,
	int32 group_id, std::string leader, std::string member, GroupOptions* options,
	std::string peerId, std::vector<int32>* raidGroups, bool is_update) {
	boost::property_tree::ptree root;
	root.put("group_id", group_id);
	root.put("leader_name", leader);
	root.put("member_name", member);
	populateGroupOptions(root, options);
	root.put("peer_web_address", peer_creation_address);
	root.put("peer_web_port", peer_creation_port);
	if (raidGroups) {
		std::vector<int32>::iterator group_itr;
		int8 group_count = 0;
		for (group_itr = raidGroups->begin(); group_itr != raidGroups->end(); group_itr++) {
			std::string fieldName("group_id_");
			fieldName.append(std::to_string(group_count));
			root.put(fieldName, (*group_itr));
			group_count++;
		}
	}
	root.put("is_update", is_update);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: ToPeer: %s (Optional), NewGroup %u, IsUpdate: %u", __FUNCTION__, peerId.c_str(), group_id, is_update);

	if (peerId.size() > 0) {
		std::shared_ptr<Peer> peer = getPeerById(peerId);

		if (peer->healthCheck.status == HealthStatus::OK) {
			peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/newgroup", jsonPayload);
		}
	}
	else {
		for (const auto& [id, peer] : peers) {
			if (peer->healthCheck.status != HealthStatus::OK)
				continue;
			if (peer->webAddr == peer_creation_address && peer->webPort == peer_creation_port) // skip peer it was created on
				continue;

			peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/newgroup", jsonPayload);
		}
	}
}

void PeerManager::sendPeersGroupMember(int32 group_id, GroupMemberInfo* info, bool is_update, std::string peerId) {
	if (!info) {
		return;
	}
	boost::property_tree::ptree root;
	root.put("group_id", group_id);
	root.put("member_name", info->name);
	root.put("is_client", info->is_client);
	root.put("zone", info->zone);
	root.put("current_hp", info->hp_current);
	root.put("max_hp", info->hp_max);
	root.put("current_power", info->power_current);
	root.put("max_power", info->power_max);
	root.put("level_current", info->level_current);
	root.put("level_max", info->level_max);
	root.put("race_id", info->race_id);
	root.put("class_id", info->class_id);
	root.put("is_leader", info->leader);
	root.put("mentor_target_char_id", info->mentor_target_char_id);
	root.put("client_peer_address", info->client_peer_address);
	root.put("client_peer_port", info->client_peer_port);
	root.put("is_raid_looter", info->is_raid_looter);
	root.put("is_update", is_update);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Send Member %s ToPeer: %s (Optional), NewGroup %u, IsUpdate: %u", __FUNCTION__, info->name.c_str(), peerId.c_str(), group_id, is_update);


	if (peerId.size() > 0) {
		std::shared_ptr<Peer> peer = getPeerById(peerId);

		if (peer->healthCheck.status == HealthStatus::OK) {
			peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/addgroupmember", jsonPayload);
		}
	}
	else {
		for (const auto& [id, peer] : peers) {
			if (peer->healthCheck.status != HealthStatus::OK)
				continue;

			peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/addgroupmember", jsonPayload);
		}
	}
}

void PeerManager::sendPeersRemoveGroupMember(int32 group_id, std::string name, int32 char_id, bool is_client) {
	boost::property_tree::ptree root;
	root.put("group_id", group_id);
	root.put("member_name", name);
	root.put("is_client", is_client);
	root.put("character_id", char_id);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Remove Member %s ToPeer: %s (Optional), Group %u", __FUNCTION__, name.c_str(), group_id);

	for (const auto& [id, peer] : peers) {
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;

		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/removegroupmember", jsonPayload);
	}
}

void PeerManager::sendPeersDisbandGroup(int32 group_id) {
	boost::property_tree::ptree root;
	root.put("group_id", group_id);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Disband Group %u", __FUNCTION__, group_id);

	for (const auto& [id, peer] : peers) {
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;

		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/disbandgroup", jsonPayload);
	}
}

bool PeerManager::sendPrimaryCreateGuildRequest(std::string guild_name, std::string leader_name, bool prompted_dialog, int32 spawnID) {
	std::shared_ptr<Peer> primary = getHealthyPrimaryPeerPtr();
	if (primary) {
		boost::property_tree::ptree root;

		root.put("peer_web_address", std::string(net.GetWebWorldAddress()));
		root.put("peer_web_port", std::to_string(net.GetWebWorldPort()));
		root.put("guild_name", guild_name);
		root.put("leader_name", leader_name);
		root.put("prompted_dialog", prompted_dialog);
		root.put("spawn_id", spawnID);

		std::ostringstream jsonStream;
		boost::property_tree::write_json(jsonStream, root);
		std::string jsonPayload = jsonStream.str();
		LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Create Guild Request %s, Leader %s", __FUNCTION__, guild_name.c_str(), leader_name.c_str());
		peer_https_pool.sendPostRequestToPeerAsync(primary->id, primary->webAddr, std::to_string(primary->webPort), "/createguild", jsonPayload);
		return true;
	}

	return false;
}


void PeerManager::sendPeersAddGuildMember(int32 character_id, int32 guild_id, std::string invited_by, int32 join_timestamp, int8 rank) {
	boost::property_tree::ptree root;
	root.put("guild_id", guild_id);
	root.put("character_id", character_id);
	root.put("invited_by", invited_by);
	root.put("join_timestamp", join_timestamp);
	root.put("rank", rank);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Add Guild Member, Guild: %u, CharID: %u, InvitedBy: %s", __FUNCTION__, guild_id, character_id, invited_by.c_str());

	for (const auto& [id, peer] : peers) {
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;
		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/addguildmember", jsonPayload);
	}
}

void PeerManager::sendPeersRemoveGuildMember(int32 character_id, int32 guild_id, std::string removed_by) {
	boost::property_tree::ptree root;
	root.put("guild_id", guild_id);
	root.put("character_id", character_id);
	root.put("removed_by", removed_by);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Remove Guild Member, Guild: %u, CharID: %u, RemovedBy: %s", __FUNCTION__, guild_id, character_id, removed_by.c_str());

	for (const auto& [id, peer] : peers) {
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;
		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/removeguildmember", jsonPayload);
	}
}

void PeerManager::sendPeersCreateGuild(int32 guild_id) {
	boost::property_tree::ptree root;
	root.put("guild_id", guild_id);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Notify Peers Guild Create, Guild: %u", __FUNCTION__, guild_id);

	for (const auto& [id, peer] : peers) {
		// primary creates the guild, skip it
		if (peer->healthCheck.status != HealthStatus::OK || peer->peeringStatus == PeeringStatus::PRIMARY)
			continue;

		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/createguild", jsonPayload);
	}
}

void PeerManager::sendPeersGuildPermission(int32 guild_id, int8 rank, int8 permission, int8 value_) {
	boost::property_tree::ptree root;
	root.put("guild_id", guild_id);
	root.put("rank", rank);
	root.put("permission", permission);
	root.put("value", value_);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Notify Peers Guild Permission, Guild: %u, Rank: %u, Permission: %u, Value: %u", __FUNCTION__, guild_id, rank, permission, value_);
	for (const auto& [id, peer] : peers) {
		// primary creates the guild, skip it
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;

		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/setguildpermission", jsonPayload);
	}
}

void PeerManager::sendPeersGuildEventFilter(int32 guild_id, int8 event_id, int8 category, int8 value_) {
	boost::property_tree::ptree root;
	root.put("guild_id", guild_id);
	root.put("event_id", event_id);
	root.put("category", category);
	root.put("value", value_);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	LogWrite(PEERING__DEBUG, 0, "Peering", "%s: Notify Peers Guild Event Filter, Guild: %u, EventID: %u, Category: %u, Value: %u", __FUNCTION__, guild_id, event_id, category, value_);

	for (const auto& [id, peer] : peers) {
		// primary creates the guild, skip it
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;

		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/setguildeventfilter", jsonPayload);
	}
}

void PeerManager::SetPeerErrorState(std::string address, std::string port) {
	std::string id = isPeer(address, (int16)std::atol(port.c_str()));
	if (id.size() > 0)
		updateHealth(id, HealthStatus::ERROR);
}

std::shared_ptr<Peer> PeerManager::getCurrentPrimary() {
	for (const auto& [id, peer] : peers) {
		if (peer->peeringStatus == PeeringStatus::PRIMARY) {
			return peer;
		}
	}
	return nullptr; // Or throw an error if no primary found
}

void PeerManager::sendPeersMessage(const std::string& endpoint, int32 command, int32 sub_command) {

	boost::property_tree::ptree root;

	root.put("reload_command", command);
	root.put("sub_command", sub_command);

	std::ostringstream jsonStream;
	boost::property_tree::write_json(jsonStream, root);
	std::string jsonPayload = jsonStream.str();
	for (const auto& [id, peer] : peers) {
		if (peer->healthCheck.status != HealthStatus::OK)
			continue;

		peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), endpoint.c_str(), jsonPayload);
	}
}