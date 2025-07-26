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

#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <iostream>
#include <chrono>
#include <map>
#include <string>
#include <optional>
#include <memory>
#include <mutex>
#include <atomic>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../../common/types.h"

class Client;

enum HealthStatus {
	UNKNOWN = 0,
	WARN = 1,
	ERROR = 2,
	STARTUP = 3,
	OK = 4,
	SHUTDOWN = 5
};

enum class PeeringStatus {
	PRIMARY,
	SECONDARY
};

struct HealthCheck {
	HealthStatus status;
	std::chrono::system_clock::time_point lastReceived;

	void updateStatus(HealthStatus newStatus);
	std::chrono::duration<double> timeSinceLastCheck() const;
};

struct GroupOptions;
struct GroupMemberInfo;
struct WhoAllPeerPlayer;
struct GuildMember;

struct ZoneChangeDetails {
	std::string peerId;
	std::string peerWorldAddress;
	std::string peerInternalWorldAddress;
	int16 peerWorldPort;
	std::string peerWebAddress;
	int16 peerWebPort;
	std::string zoneFileName;
	std::string zoneName;
	int32 zoneId;
	int32 instanceId;
	float safeX;
	float safeY;
	float safeZ;
	float safeHeading;
	bool lockState;
	sint16 minStatus;
	int16 minLevel;
	int16 maxLevel;
	int16 minVersion;

	int32 defaultLockoutTime;
	int32 defaultReenterTime;
	int8 instanceType;
	int32 numPlayers;
	bool isCityZone;
	void* zonePtr;
	bool peerAuthorized;
	int32 zoneKey;
	int32 authDispatchedTime;
	bool zoningPastAuth;
	ZoneChangeDetails() = default;
	ZoneChangeDetails(ZoneChangeDetails* copy_details);
	ZoneChangeDetails(std::string peer_id, std::string peer_world_address, std::string peer_internal_world_address, int16 peer_world_port,
		std::string peer_web_address, int16 peer_web_port, std::string zone_file_name, std::string zone_name, int32 zone_id,
		int32 instance_id, float safe_x, float safe_y, float safe_z, float safe_heading, bool lock_state, sint16 min_status,
		int16 min_level, int16 max_level, int16 min_version, int32 default_lockout_time, int32 default_reenter_time, int8 instance_type, int32 num_players);
};

struct Peer {
	std::string id;
	PeeringStatus peeringStatus;
	HealthCheck healthCheck;
	std::string worldAddr;
	std::string internalWorldAddr;
	int16 worldPort;
	int16 peerPriority;
	std::string webAddr;
	int16 webPort;
	std::shared_ptr<boost::property_tree::ptree> zone_tree;
	std::shared_ptr<boost::property_tree::ptree> client_tree;
	std::atomic<bool> sentInitialPeerData;
	std::atomic<bool> wasOffline;
	mutable std::mutex dataMutex;  // Mutex to protect access to ptree
	// Default constructor
	Peer()
		: zone_tree(std::make_shared<boost::property_tree::ptree>()),
		client_tree(std::make_shared<boost::property_tree::ptree>()) {
		healthCheck.status = HealthStatus::UNKNOWN;
		peerPriority = 65535;
		sentInitialPeerData = false;
		wasOffline = false;
	}

	Peer(std::string peerId, PeeringStatus status, std::string client_address,
		std::string client_internal_address, int16 client_port,
		std::string web_address, int16 web_port)
		: id(std::move(peerId)), peeringStatus(status), worldAddr(std::move(client_address)),
		internalWorldAddr(std::move(client_internal_address)), worldPort(client_port),
		webAddr(std::move(web_address)), webPort(web_port),
		zone_tree(std::make_shared<boost::property_tree::ptree>()),
		client_tree(std::make_shared<boost::property_tree::ptree>()) {
		healthCheck.status = HealthStatus::STARTUP;
		peerPriority = 65535;
		sentInitialPeerData = false;
		wasOffline = false;
	}

	// Example function to output data as JSON string (for debug or logging)
	std::string getZoneDataAsJson() const {
		std::lock_guard<std::mutex> lock(dataMutex);
		std::ostringstream oss;
		boost::property_tree::write_json(oss, *zone_tree);  // Dereference data
		return oss.str();
	}

	// Example function to output data as JSON string (for debug or logging)
	std::string getClientDataAsJson() const {
		std::lock_guard<std::mutex> lock(dataMutex);
		std::ostringstream oss;
		boost::property_tree::write_json(oss, *client_tree);  // Dereference data
		return oss.str();
	}
};

class PeerManager {
private:
	std::map<std::string, std::shared_ptr<Peer>> peers;
	std::atomic<int32> uniqueGroupID{ 1 };  // Shared counter for unique IDs
	std::mutex idMutex;

public:
	void addPeer(std::string id, PeeringStatus status, std::string client_address, std::string client_internal_address, int16 client_port, std::string web_address, int16 web_port);
	void updateHealth(const std::string& id, HealthStatus newStatus);
	void updatePriority(const std::string& id, int16 priority);
	void updateZoneTree(const std::string& id, const boost::property_tree::ptree& newTree);
	void updateClientTree(const std::string& id, const boost::property_tree::ptree& newTree);
	void setZonePeerData(ZoneChangeDetails* opt_details, std::string peerId, std::string peerWorldAddress, std::string peerInternalWorldAddress, int16 peerWorldPort,
		std::string peerWebAddress, int16 peerWebPort, std::string zoneFileName, std::string zoneName, int32 zoneId,
		int32 instanceId, float safeX, float safeY, float safeZ, float safeHeading, bool lockState, sint16 minStatus,
		int16 minLevel, int16 maxLevel, int16 minVersion, int32 defaultLockoutTime, int32 defaultReenterTime, int8 instanceType, int32 numPlayers, bool isCityZone);
	void setZonePeerDataSelf(ZoneChangeDetails* opt_details, std::string zoneFileName, std::string zoneName, int32 zoneId,
		int32 instanceId, float safeX, float safeY, float safeZ, float safeHeading, bool lockState, sint16 minStatus,
		int16 minLevel, int16 maxLevel, int16 minVersion, int32 defaultLockoutTime, int32 defaultReenterTime, int8 instanceType, int32 numPlayers, bool isCityZone, void* zonePtr = nullptr);
	bool IsClientConnectedPeer(int32 account_id, int32 char_id);
	std::string GetCharacterPeerId(std::string charName);
	void SendPeersChannelMessage(int32 group_id, std::string fromName, std::string message, int16 channel, int32 language_id = 0, int8 custom_type = 0);
	void SendPeersGuildChannelMessage(int32 guild_id, std::string fromName, std::string message, int16 channel, int32 language_id = 0, int8 custom_type = 0);
	void sendZonePeerList(Client* client);
	std::string getZonePeerId(const std::string& inc_zone_name, int32 inc_zone_id, int32 inc_instance_id, ZoneChangeDetails* opt_details = nullptr, bool only_always_loaded = false, int32 matchDuplicatedId = 0);
	int32 getZoneHighestDuplicateId(const std::string& inc_zone_name, int32 inc_zone_id, bool increment_new_value = true);
	void setZonePeerData(ZoneChangeDetails* opt_details);
	void setPrimary(const std::string& id);
	bool hasPrimary();
	bool hasPriorityPeer(int16 priority);
	std::string getPriorityPeer();
	void updatePeer(const std::string& web_address, int16 web_port, const std::string& client_address, const std::string& client_internal_address, int16 client_port, bool is_primary = false);
	std::string isPeer(const std::string& web_address, int16 web_port);
	HealthStatus getPeerStatus(const std::string& web_address, int16 web_port);
	bool hasPeers();
	std::string assignUniqueNameForSecondary(const std::string& baseName, std::string client_address, std::string client_internal_address, int16 client_port, std::string web_address, int16 web_port);
	std::optional<std::string> getHealthyPeer() const;
	std::shared_ptr<Peer> getHealthyPeerPtr() const;
	std::shared_ptr<Peer> getHealthyPrimaryPeerPtr() const;
	std::shared_ptr<Peer> getHealthyPeerWithLeastClients() const;
	std::shared_ptr<Peer> getPeerById(const std::string& id) const;
	int32 getUniqueGroupId();
	bool sendPrimaryNewGroupRequest(std::string leader, std::string member, int32 entity_id, GroupOptions* options);
	void sendPeersGroupMember(int32 group_id, GroupMemberInfo* info, bool is_update = false, std::string peerId = "");
	void sendPeersRemoveGroupMember(int32 group_id, std::string name, int32 char_id, bool is_client);
	void populateGroupOptions(boost::property_tree::ptree& root, GroupOptions* options);
	void sendPeersNewGroupRequest(std::string peer_creation_address, int16 peer_creation_port,
		int32 group_id, std::string leader, std::string member, GroupOptions* options,
		std::string peerId = "", std::vector<int32>* raidGroups = nullptr, bool is_update = false);

	void sendPeersDisbandGroup(int32 group_id);
	bool sendPrimaryCreateGuildRequest(std::string guild_name, std::string leader_name, bool prompted_dialog = false, int32 spawnID = 0);
	void sendPeersAddGuildMember(int32 character_id, int32 guild_id, std::string invited_by, int32 join_timestamp, int8 rank);
	void sendPeersRemoveGuildMember(int32 character_id, int32 guild_id, std::string removed_by);
	void sendPeersCreateGuild(int32 guild_id);
	void sendPeersGuildPermission(int32 guild_id, int8 rank, int8 permission, int8 value_);
	void sendPeersGuildEventFilter(int32 guild_id, int8 event_id, int8 category, int8 value_);

	void SetPeerErrorState(std::string address, std::string port);
	void handlePrimaryConflict(const std::string& reconnectingPeerId);
	std::shared_ptr<Peer> getCurrentPrimary();

	void sendPeersMessage(const std::string& endpoint, int32 command, int32 sub_command = 0);
	void sendPeersActiveQuery(int32 character_id, bool remove = false);

	void sendZonePlayerList(std::vector<string>* queries, std::vector<WhoAllPeerPlayer>* peer_list, bool isGM);

	bool GetClientGuildDetails(int32 matchCharID, GuildMember* member_details);
	
	
	void sendPeersAddSeller(int32 character_id, int32 house_id, std::string name, bool saleEnabled, bool invEnabled);
	void sendPeersRemoveSeller(int32 character_id);
	void sendPeersAddItemSale(int32 character_id, int32 house_id, int32 itemID, int64 uniqueID, int64 price, sint32 invSlotID, int16 slotID, int16 count, bool inInventory, bool forSale, std::string itemCreator);
	void sendPeersRemoveItemSale(int32 character_id, int64 uniqueID);

};

#endif // PEERMANAGER_H
