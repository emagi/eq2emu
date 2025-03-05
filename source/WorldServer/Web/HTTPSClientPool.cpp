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

#include "HTTPSClientPool.h"
#include "PeerManager.h"
#include "../net.h"
#include "../World.h"
#include "../LoginServer.h"
#include "../WorldDatabase.h"
#include "../Guilds/Guild.h"
#include "../../common/Log.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <regex>
#include <iostream>
#include <sstream>

extern NetConnection net;
extern PeerManager peer_manager;
extern World world;
extern LoginServer loginserver;
extern ZoneList zone_list;
extern WorldDatabase database;
extern GuildList guild_list;
extern HTTPSClientPool peer_https_pool;

// Handler functions for each endpoint
void AddCharAuth(boost::property_tree::ptree tree) {
	int32 success = 0;
	std::string charName(""), zoneName("");
	int32 acct_id = 0;
	int32 zoneId = 0, instanceId = 0;
	bool firstLogin = false;
	std::string worldAddr(""), internalWorldAddr(""), clientIP("");
	int16 worldPort = 0;
	int32 charID = 0;
	int32 worldID = 0, fromID = 0;
	int32 loginKey = 0;
	if (auto char_name = tree.get_optional<std::string>("character_name")) {
		charName = char_name.get();
	}
	if (auto successful = tree.get_optional<int32>("success")) {
		success = successful.get();
	}
	if (auto account_id = tree.get_optional<int32>("account_id")) {
		acct_id = account_id.get();
	}
	if (auto zone_name = tree.get_optional<std::string>("zone_name")) {
		zoneName = zone_name.get();
	}
	if (auto zone_id = tree.get_optional<int32>("zone_id")) {
		zoneId = zone_id.get();
	}
	if (auto instance_id = tree.get_optional<int32>("instance_id")) {
		instanceId = instance_id.get();
	}
	if (auto first_login = tree.get_optional<bool>("first_login")) {
		firstLogin = first_login.get();
	}

	if (auto peerclientaddr = tree.get_optional<std::string>("peer_client_address")) {
		worldAddr = peerclientaddr.get();
	}
	if (auto peerclient_internaladdr = tree.get_optional<std::string>("peer_client_internal_address")) {
		internalWorldAddr = peerclient_internaladdr.get();
	}
	if (auto peerclientport = tree.get_optional<int16>("peer_client_port")) {
		worldPort = peerclientport.get();
	}
	if (auto clientip = tree.get_optional<std::string>("client_ip")) {
		clientIP = clientip.get();
	}
	if (auto character_id = tree.get_optional<int32>("character_id")) {
		charID = character_id.get();
	}
	if (auto login_key = tree.get_optional<int32>("login_key")) {
		loginKey = login_key.get();
	}
	if (auto world_id = tree.get_optional<int32>("world_id")) {
		worldID = world_id.get();
	}
	if (auto from_id = tree.get_optional<int32>("from_id")) {
		fromID = from_id.get();
	}

	LogWrite(PEERING__INFO, 0, "Peering", "%s: Handling AddCharAuth %s (%u) for zone %u instance %u", __FUNCTION__, charName.c_str(), charID, zoneId, instanceId);

	if (firstLogin) {
		loginserver.SendCharApprovedLogin(success, worldAddr, internalWorldAddr, clientIP, worldPort, acct_id, charID, loginKey, worldID, fromID);
	}
	else if (acct_id > 0 && charName.length() > 0) {
		world.ClientAuthApproval(success, charName, acct_id, zoneName, zoneId, instanceId, firstLogin);
	}
}

void StartZone(boost::property_tree::ptree tree) {
	std::string peerWebAddress("");
	int16 peerWebPort = 0;
	if (auto addr = tree.get_optional<std::string>("peer_web_address")) {
		peerWebAddress = addr.get();
	}
	if (auto port = tree.get_optional<int16>("peer_web_port")) {
		peerWebPort = port.get();
	}

	std::string id = peer_manager.isPeer(peerWebAddress, peerWebPort);

	if (id.size() > 0) {
		std::string strPort = std::to_string(peerWebPort);
		auto client = peer_https_pool.getOrCreateClient(id, peerWebAddress, strPort);
		if (client) {
			auto responseZones = client->sendRequest(peerWebAddress, strPort, "/zones");  // Assumes HTTPSClient has a get method
			// Load the JSON data into the property tree
			std::istringstream json_stream(responseZones);
			if (json_stream.str().empty()) {
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: JSON Stream Empty for %s:%s/zones", __FUNCTION__, peerWebAddress.c_str(), strPort.c_str());
			}
			else if (json_stream.fail()) {
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: JSON Failed State for %s:%s/zones", __FUNCTION__, peerWebAddress.c_str(), strPort.c_str());
			}
			else if (json_stream.bad()) {
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: JSON Stream Bad for %s:%s/zones", __FUNCTION__, peerWebAddress.c_str(), strPort.c_str());
			}
			else if (json_stream.eof()) {
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: JSON Stream EOF for %s:%s/zones", __FUNCTION__, peerWebAddress.c_str(), strPort.c_str());
			}
			else {
				boost::property_tree::ptree pt;
				boost::property_tree::read_json(json_stream, pt);
				peer_manager.updateZoneTree(id, pt);
				LogWrite(PEERING__DEBUG, 5, "Peering", "%s: StartZone Update for %s:%s/zones complete, zone tree updated.", __FUNCTION__, peerWebAddress.c_str(), strPort.c_str());
			}
		}
	}
}

void SendGlobalMessage(boost::property_tree::ptree tree) {
	int32 success = 0;
	int8 language = 0;
	int16 in_channel = 0;
	std::string toName(""), fromName(""), msg(""), awaymsg("");
	int8 away_language = 0;
	int32 group_id = 0;
	if (auto successful = tree.get_optional<int32>("success")) {
		success = successful.get();
	}
	if (auto name = tree.get_optional<std::string>("to_name")) {
		toName = name.get();
	}
	if (auto name = tree.get_optional<std::string>("from_name")) {
		fromName = name.get();
	}
	if (auto message = tree.get_optional<std::string>("message")) {
		msg = message.get();
	}
	if (auto from_language = tree.get_optional<int8>("from_language")) {
		language = from_language.get();
	}
	if (auto channel = tree.get_optional<int16>("channel")) {
		in_channel = channel.get();
	}

	if (auto msg = tree.get_optional<std::string>("away_message")) {
		awaymsg = msg.get();
	}
	if (auto away_lang = tree.get_optional<int8>("away_language")) {
		away_language = away_lang.get();
	}
	if (auto group = tree.get_optional<int32>("group_id")) {
		group_id = group.get();
	}

	switch (in_channel) {
	case CHANNEL_PRIVATE_TELL: {
		Client* from_client = zone_list.GetClientByCharName(fromName.c_str());
		if (from_client) {
			if (success) {
				from_client->HandleTellMessage(from_client->GetPlayer()->GetName(), msg.c_str(), toName.c_str(), language);
				if (awaymsg.size() > 0) {
					from_client->HandleTellMessage(toName.c_str(), awaymsg.c_str(), from_client->GetPlayer()->GetName(), away_language);
				}
			}
			else {
				from_client->SimpleMessage(CHANNEL_COLOR_CHAT_RELATIONSHIP, "That character does not exist.");
			}
		}
		break;
	}
	}
}

void HandleNewGroup(boost::property_tree::ptree tree) {

	int32 success = 0;
	if (auto successful = tree.get_optional<int32>("success")) {
		success = successful.get();
	}
	if (!net.is_primary) {
		// we are a peer we ask primary for a new group and get the response
		GroupOptions options;
		std::string leader(""), member("");
		int32 group_id = 0;
		int32 member_entity_id = 0;
		if (auto leader_name = tree.get_optional<std::string>("leader_name")) {
			leader = leader_name.get();
		}
		if (auto member_name = tree.get_optional<std::string>("member_name")) {
			member = member_name.get();
		}
		if (auto lootmethod = tree.get_optional<int8>("loot_method")) {
			options.loot_method = lootmethod.get();
		}
		if (auto itemrarity = tree.get_optional<int8>("loot_item_rarity")) {
			options.loot_items_rarity = itemrarity.get();
		}
		if (auto autosplit = tree.get_optional<int8>("auto_split")) {
			options.auto_split = autosplit.get();
		}
		if (auto defaultyell = tree.get_optional<int8>("default_yell")) {
			options.default_yell = defaultyell.get();
		}
		if (auto grouplockmethod = tree.get_optional<int8>("group_lock_method")) {
			options.group_lock_method = grouplockmethod.get();
		}
		if (auto groupautolock = tree.get_optional<int8>("group_auto_lock")) {
			options.group_autolock = groupautolock.get();
		}
		if (auto soloautolock = tree.get_optional<int8>("solo_auto_lock")) {
			options.solo_autolock = soloautolock.get();
		}
		if (auto autoloot = tree.get_optional<int8>("auto_loot_method")) {
			options.auto_loot_method = autoloot.get();
		}
		if (auto lastlootindex = tree.get_optional<int8>("last_looted_index")) {
			options.last_looted_index = lastlootindex.get();
		}
		if (auto groupid = tree.get_optional<int32>("group_id")) {
			group_id = groupid.get();
		}

		if (auto entityid = tree.get_optional<int32>("member_entity_id")) {
			member_entity_id = entityid.get();
		}

		LogWrite(PEERING__INFO, 0, "Peering", "%s: Add New Group %u with member %s (%u) leader %s", __FUNCTION__, group_id, member.c_str(), member_entity_id, leader.c_str());
		Client* member_client = zone_list.GetClientByCharName(member.c_str());
		Client* client_leader = zone_list.GetClientByCharName(leader.c_str());
		if (success) {
			if (!member_client) {
				if (client_leader && client_leader->GetPlayer()->GetZone()) {
					Spawn* spawn = client_leader->GetPlayer()->GetZone()->GetSpawnByID(member_entity_id);
					if (spawn && spawn->IsEntity()) {
						world.GetGroupManager()->AcceptInvite((Entity*)spawn, &group_id);
					}
					else {
						LogWrite(PEERING__ERROR, 0, "Peering", "%s: Failed Adding New Group %u, entity %s (%u) did not exist.", __FUNCTION__, group_id, member.c_str(), member_entity_id);
					}
				}
				else {
					LogWrite(PEERING__ERROR, 0, "Peering", "%s: Failed Adding New Group %u, member %s (%u) did not exist.", __FUNCTION__, group_id, member.c_str(), member_entity_id);
				}
			}
			else if (!client_leader) {
				if (member_client)
					member_client->HandleGroupAcceptResponse(2);
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: Failed Adding New Group %u, leader %s did not exist.", __FUNCTION__, group_id, leader.c_str());
			}
			else {
				world.GetGroupManager()->AcceptInvite(member_client->GetPlayer(), &group_id);
			}
		}
	}
}


void HandleCreateGuild(boost::property_tree::ptree tree) {

	bool success = false;
	int32 guildID = 0;
	std::string leaderName("");
	bool promptedDialog = false;
	int32 spawnID = 0;
	if (auto successful = tree.get_optional<bool>("success")) {
		success = successful.get();
	}

	if (auto guild_id = tree.get_optional<int32>("guild_id")) {
		guildID = guild_id.get();
	}

	if (auto name = tree.get_optional<std::string>("leader_name")) {
		leaderName = name.get();
	}

	if (auto prompt = tree.get_optional<bool>("prompted_dialog")) {
		promptedDialog = prompt.get();
	}

	if (auto spawnid = tree.get_optional<int32>("spawn_id")) {
		spawnID = spawnid.get();
	}

	if (net.is_primary) {
		// we send out to peers 
	}
	else if (guildID) {
		database.LoadGuild(guildID);
		Guild* guild = guild_list.GetGuild(guildID);
		Client* leader = zone_list.GetClientByCharName(leaderName.c_str());
		if (leader && guild && !leader->GetPlayer()->GetGuild()) {
			if(spawnID) {
				Spawn* npc = leader->GetPlayer()->GetZone()->GetSpawnByID(spawnID);
				if(promptedDialog && npc && npc->IsNPC()) {
					leader->GetCurrentZone()->CallSpawnScript(npc, SPAWN_SCRIPT_CASTED_ON, leader->GetPlayer(), guild->GetName());
				}
			}
			guild->AddNewGuildMember(leader, 0, GUILD_RANK_LEADER);
			database.SaveGuildMembers(guild);
			if (leader && leader->GetPlayer()->GetGroupMemberInfo()) {
				world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

				PlayerGroup* group = world.GetGroupManager()->GetGroup(leader->GetPlayer()->GetGroupMemberInfo()->group_id);
				if (group)
				{
					GroupMemberInfo* gmi = nullptr;
					deque<GroupMemberInfo*>::iterator itr;
					group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
					deque<GroupMemberInfo*>* members = group->GetMembers();
					for (itr = members->begin(); itr != members->end(); itr++) {
						gmi = *itr;
						if (gmi->client && gmi->client != leader && !gmi->client->GetPlayer()->GetGuild())
							guild->InvitePlayer(gmi->client, leader->GetPlayer()->GetName());
					}
					group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
				}

				world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
			}
		}
	}
}

// Define a type for handler functions
using HandlerFunction = std::function<void(boost::property_tree::ptree tree)>;

// Set up the endpoint-to-function map
std::unordered_map<std::string, HandlerFunction> endpointHandlers = {
	{"/addcharauth", AddCharAuth},
	{"/startzone", StartZone},
	{"/sendglobalmessage", SendGlobalMessage},
	{"/newgroup", HandleNewGroup},
	{"/createguild", HandleCreateGuild}
};

HTTPSClientPool::HTTPSClientPool() {}

void HTTPSClientPool::init(const std::string& cert, const std::string& key) {
	certFile = cert;
	keyFile = key;
	pollingInterval = 1000;

	int32 numThreads = 2;
	// Start worker threads
	for (int32 i = 0; i < numThreads; ++i) {
		workers.emplace_back(&HTTPSClientPool::workerFunction, this);
	}
}

HTTPSClientPool::~HTTPSClientPool() {
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread& worker : workers) {
		if (worker.joinable()) {
			worker.join();
		}
	}
}

void HTTPSClientPool::addPeerClient(const std::string& peerId, const std::string& server, const std::string& port, const std::string& authEndpoint) {
	bool newClient = false;
	// Check if client already exists for the specified (server, port) pair
	auto it = clients.find(std::make_pair(server, port));
	if (it == clients.end()) {
		newClient = true;
	}

	if (newClient) {
		auto client = getOrCreateClient(peerId, server, port);
	}
}

boost::property_tree::ptree HTTPSClientPool::sendRequestToPeer(const std::string& peerId, const std::string& target) {
	auto client = getClient(peerId);
	boost::property_tree::ptree pt;

	if (!client) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Client for peer %s could not be found.", __FUNCTION__, peerId.c_str());
		return pt;
	}

	// Retrieve the response from the client
	std::string responseBody = client->sendRequest(client->getServer(), client->getPort(), target);

	if (responseBody.size() < 1) {

		int16 peer_port = 0;
		try {
			peer_port = std::stoul(client->getPort());
		}
		catch (const std::exception& e) {
			LogWrite(PEERING__ERROR, 0, "Peering", "%s: Error for convering peer port for %s: %s.", __FUNCTION__, peerId.c_str(), e.what() ? e.what() : "??");
		}
		HealthStatus curStatus = peer_manager.getPeerStatus(client->getServer(), peer_port);
		switch (curStatus) {
		case HealthStatus::WARN: {
			peer_manager.updateHealth(peerId, HealthStatus::ERROR);
			break;
		}
		case HealthStatus::ERROR: {
			peer_manager.updateHealth(peerId, HealthStatus::SHUTDOWN);
			break;
		}
		default: {
			peer_manager.updateHealth(peerId, HealthStatus::WARN);
			break;
		}
		}
	}
	else {
		// Parse the response as JSON
		try {
			std::istringstream responseStream(responseBody);
			boost::property_tree::read_json(responseStream, pt);
		}
		catch (const boost::property_tree::json_parser_error& e) {
			LogWrite(PEERING__ERROR, 0, "Peering", "%s: JSON Parsing error for %s: %s.", __FUNCTION__, peerId.c_str(), e.what() ? e.what() : "??");
		}
	}

	return pt;
}

boost::property_tree::ptree HTTPSClientPool::sendPostRequestToPeer(const std::string& peerId, const std::string& target, const std::string& jsonPayload) {
	auto client = getClient(peerId);
	boost::property_tree::ptree pt;

	if (!client) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Client for peer %s could not be found.", __FUNCTION__, peerId.c_str());
		return pt;
	}

	// Retrieve the response from the client
	std::string responseBody = client->sendPostRequest(client->getServer(), client->getPort(), target, jsonPayload);

	// Parse the response as JSON
	try {
		std::istringstream responseStream(responseBody);
		boost::property_tree::read_json(responseStream, pt);
	}
	catch (const boost::property_tree::json_parser_error& e) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Client for peer %s could not be found.", __FUNCTION__, peerId.c_str());
	}

	return pt;
}

std::shared_ptr<HTTPSClient> HTTPSClientPool::getClient(const std::string& peerId) {
	std::unique_lock<std::mutex> lock(queueMutex);
	// Lookup the client by ID
	auto idIt = clientsById.find(peerId);
	if (idIt != clientsById.end()) {
		return idIt->second;
	}
	return nullptr;  // Return nullptr if no client exists with the given ID
}

std::shared_ptr<HTTPSClient> HTTPSClientPool::getOrCreateClient(const std::string& id, const std::string& server, const std::string& port) {
	std::unique_lock<std::mutex> lock(queueMutex);
	// Create a key based on (server, port)
	auto clientKey = std::make_pair(server, port);

	// Check if client already exists for the specified (server, port) pair
	auto it = clients.find(clientKey);
	if (it != clients.end()) {
		// Client already exists, return the existing client
		return it->second;
	}

	// No existing client for this (server, port), create and store a new HTTPSClient
	auto client = std::make_shared<HTTPSClient>(certFile, keyFile);
	clients[clientKey] = client;
	clientsById[id] = client;

	return client;
}

void HTTPSClientPool::pollPeerHealth(const std::string& server, const std::string& port) {
	int16 web_worldport = 0;
	try {
		web_worldport = std::stoul(port);
	}
	catch (const std::exception& e) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Getting port for peer %s:%s could not be complete.", __FUNCTION__, server.c_str(), port.c_str());
	}
	std::string id = peer_manager.isPeer(server, web_worldport);
	if (id.size() < 1) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Error finding peer %s:%s.", __FUNCTION__, server.c_str(), port.c_str());
	}
	else {
		auto client = getOrCreateClient(id, server, port);
		int16 interval = pollingInterval;
		HealthStatus curStatus = peer_manager.getPeerStatus(server, web_worldport);
		id = peer_manager.isPeer(server, web_worldport);
		try {
			auto response = client->sendRequest(server, port, "/peerstatus");  // Assumes HTTPSClient has a get method
			//std::cout << "Health check response from " << server << ":" << port << " - " << response << std::endl;

			boost::property_tree::ptree json_tree;
			std::istringstream json_stream(response);
			boost::property_tree::read_json(json_stream, json_tree);
			std::string online_status;
			int16 peer_priority = 65535;
			bool peer_primary = false;
			
			std::string worldAddr(""), internalWorldAddr(""), clientIP("");
			int16 worldPort = 0;
			try {
				if (auto status = json_tree.get_optional<std::string>("world_status")) {
					online_status = status.get();
				}
				if (auto priority = json_tree.get_optional<int16>("peer_priority")) {
					peer_priority = priority.get();
				}
				if (auto isprimary = json_tree.get_optional<bool>("peer_primary")) {
					peer_primary = isprimary.get();
				}
				if (auto peerclientaddr = json_tree.get_optional<std::string>("peer_client_address")) {
					worldAddr = peerclientaddr.get();
				}
				if (auto peerclient_internaladdr = json_tree.get_optional<std::string>("peer_client_internal_address")) {
					internalWorldAddr = peerclient_internaladdr.get();
				}
				if (auto peerclientport = json_tree.get_optional<int16>("peer_client_port")) {
					worldPort = peerclientport.get();
				}
				if(worldAddr.size() > 0 && worldPort > 0) {
					peer_manager.updatePeer(server, web_worldport, worldAddr, internalWorldAddr, worldPort, peer_primary);
				}
				peer_manager.updatePriority(id, peer_priority);
			} catch (const boost::property_tree::ptree_error& e) {
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: Error accessing WebStatus tree: Peer %s at %s:%s - HAS PRIMARY status, demoting self.", __FUNCTION__, id.c_str(), server.c_str(), port.c_str());
			}
			// Process Clients
			if (json_tree.find("Clients") != json_tree.not_found()) {
				// Create a new ptree to preserve the "Zones" key
				boost::property_tree::ptree clients_copy;

				// Copy the "Zones" node to zones_copy to preserve its key
				clients_copy.put_child("Clients", json_tree.get_child("Clients"));
				peer_manager.updateClientTree(id, clients_copy);
			}
			
			// Process Zones
			if (json_tree.find("Zones") != json_tree.not_found()) {
				// Create a new ptree to preserve the "Zones" key
				boost::property_tree::ptree zones_copy;

				// Copy the "Zones" node to zones_copy to preserve its key
				zones_copy.put_child("Zones", json_tree.get_child("Zones"));
				peer_manager.updateZoneTree(id, zones_copy);
			}
			
			if (peer_primary && net.is_primary) {
				peer_manager.handlePrimaryConflict(id);
				std::shared_ptr<Peer> hasPrimary = peer_manager.getHealthyPrimaryPeerPtr();
				if (hasPrimary) { // demote self
					LogWrite(PEERING__INFO, 0, "Peering", "%s: Peer %s at %s:%s - HAS PRIMARY status, demoting self.", __FUNCTION__, id.c_str(), server.c_str(), port.c_str());
					net.SetPrimary(false);
				}
			}
			else if (!peer_manager.hasPrimary() && peer_primary) {
				peer_manager.setPrimary(id);
			}

			switch (curStatus) {
			case HealthStatus::STARTUP: {
				if (online_status == "offline") {
					std::shared_ptr<Peer> peer = peer_manager.getPeerById(id);
					if (peer) {
						peer->wasOffline = true;
						if (peer->sentInitialPeerData) {
							peer->sentInitialPeerData = false;
						}
					}
				}
				if (online_status == "online") {
					peer_manager.updateHealth(id, HealthStatus::OK);
					std::shared_ptr<Peer> peer = peer_manager.getPeerById(id);
					if (net.is_primary) {
						if (peer) {
							if (peer->wasOffline && !peer->sentInitialPeerData) {
								world.GetGroupManager()->SendPeerGroupData(id);
							}
							peer->sentInitialPeerData = true;
						}
					}
					else if (peer) { // set as if we already sent the data since if we take over we don't want the peer trying to resubmit all groups
						peer->wasOffline = false;
						peer->sentInitialPeerData = true;
					}
					LogWrite(PEERING__INFO, 0, "Peering", "%s: Peer %s at %s:%s - HAS OK/UP state.", __FUNCTION__, id.c_str(), server.c_str(), port.c_str());
				}
				if (!net.is_primary && !peer_manager.hasPrimary() && peer_priority < net.GetPeerPriority()) {
					LogWrite(PEERING__INFO, 0, "Peering", "%s: Peer %s at %s:%s - HAS PRIMARY.", __FUNCTION__, id.c_str(), server.c_str(), port.c_str());
					peer_manager.setPrimary(id);
					net.SetPrimary(false);
				}
				else if (!peer_manager.hasPrimary() && world.world_loaded && !net.is_primary && net.GetPeerPriority() <= peer_priority) {
					LogWrite(PEERING__INFO, 0, "Peering", "%s: I AM PRIMARY!", __FUNCTION__);
					net.SetPrimary();
				}
				break;
			}
			case HealthStatus::OK: {
				if (!net.is_primary && !peer_manager.hasPrimary() && peer_priority < net.GetPeerPriority()) {
					LogWrite(PEERING__INFO, 0, "Peering", "%s: Peer %s at %s:%s - HAS PRIMARY.", __FUNCTION__, id.c_str(), server.c_str(), port.c_str());
					peer_manager.setPrimary(id);
					net.SetPrimary(false);
				}
				else if (!peer_manager.hasPrimary() && !net.is_primary && net.GetPeerPriority() <= peer_priority) {
					LogWrite(PEERING__INFO, 0, "Peering", "%s: I AM PRIMARY!!", __FUNCTION__);
					net.SetPrimary();
				}
				break;
			}
			case HealthStatus::WARN:
			case HealthStatus::ERROR:
			case HealthStatus::SHUTDOWN: {
				peer_manager.updateHealth(id, HealthStatus::STARTUP);
				LogWrite(PEERING__INFO, 0, "Peering", "%s: Peer %s at %s:%s - HAS ENTERED STARTUP state.", __FUNCTION__, id.c_str(), server.c_str(), port.c_str());

				if (net.is_primary) {
					std::shared_ptr<Peer> peer = peer_manager.getPeerById(id);
					if (peer && peer->sentInitialPeerData == true) {
						peer->sentInitialPeerData = false;
					}
				}
				break;
			}
			}
		}
		catch (const std::exception& e) {
			HealthStatus curStatus = peer_manager.getPeerStatus(server, web_worldport);
			switch (curStatus) {
			case HealthStatus::WARN: {
				peer_manager.updateHealth(id, HealthStatus::ERROR);
				break;
			}
			case HealthStatus::ERROR: {

				LogWrite(PEERING__ERROR, 0, "Peering", "%s: Peer %s at %s:%s - HAS ERROR->SHUTDOWN state.", __FUNCTION__, id.c_str(), server.c_str(), port.c_str());
				peer_manager.updateHealth(id, HealthStatus::SHUTDOWN);
				if (peer_manager.getHealthyPeer() == std::nullopt) {
					if (!net.is_primary && world.world_loaded) {
						LogWrite(PEERING__INFO, 0, "Peering", "%s: TAKING OVER AS PRIMARY, NO PEERS AVAILABLE TO CHECK", __FUNCTION__);
						net.SetPrimary();
					}
				}
				else if (!peer_manager.hasPrimary()) {
					std::string newPrimary = peer_manager.getPriorityPeer();
					if (newPrimary.size() > 0) {
						LogWrite(PEERING__INFO, 0, "Peering", "%s: NEW PRIMARY %s", __FUNCTION__, newPrimary.c_str());
						peer_manager.setPrimary(newPrimary);
						net.SetPrimary(false);
					}
					else {
						LogWrite(PEERING__ERROR, 0, "Peering", "%s: NEW PRIMARY CANNOT BE ESTABLISHED!", __FUNCTION__);
					}
				}
				break;
			}
			default: {
				peer_manager.updateHealth(id, HealthStatus::WARN);
				break;
			}
			}
			LogWrite(PEERING__ERROR, 0, "Peering", "%s: ERROR POLLING %s:%s reason: %s", __FUNCTION__, server.c_str(), port.c_str(), e.what() ? e.what() : "??");
		}
		interval = 0;
	}
}

void HTTPSClientPool::pollPeerHealthData(std::shared_ptr<HTTPSClient> client, const std::string& id, const std::string& server, const std::string& port) {
	if (client == nullptr) {

	}
	auto responseZones = client->sendRequest(server, port, "/zones");  // Assumes HTTPSClient has a get method
	// Load the JSON data into the property tree
	std::istringstream json_stream(responseZones);
	if (json_stream.str().empty()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Stream Empty for %s:%s/zones", __FUNCTION__, server.c_str(), port.c_str());
	}
	else if (json_stream.fail()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Failed State for %s:%s/zones", __FUNCTION__, server.c_str(), port.c_str());
	}
	else if (json_stream.bad()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Stream Bad for %s:%s/zones", __FUNCTION__, server.c_str(), port.c_str());
	}
	else if (json_stream.eof()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Stream EOF for %s:%s/zones", __FUNCTION__, server.c_str(), port.c_str());
	}
	else {
		boost::property_tree::ptree pt;
		boost::property_tree::read_json(json_stream, pt);
		peer_manager.updateZoneTree(id, pt);
		LogWrite(PEERING__DEBUG, 5, "Peering", "%s: Polling for %s:%s/zones complete, zone tree updated.", __FUNCTION__, server.c_str(), port.c_str());
	}
	auto responseClients = client->sendRequest(server, port, "/clients");  // Assumes HTTPSClient has a get method

	// Load the JSON data into the property tree
	std::istringstream json_stream2(responseClients);
	if (json_stream2.str().empty()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Stream Empty for %s:%s/clients", __FUNCTION__, server.c_str(), port.c_str());
	}
	else if (json_stream2.fail()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Failed State for %s:%s/clients", __FUNCTION__, server.c_str(), port.c_str());
	}
	else if (json_stream2.bad()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Stream Bad for %s:%s/clients", __FUNCTION__, server.c_str(), port.c_str());
	}
	else if (json_stream2.eof()) {
		LogWrite(PEERING__ERROR, 0, "Peering", "%s: Polling JSON Stream EOF for %s:%s/clients", __FUNCTION__, server.c_str(), port.c_str());
	}
	else {
		boost::property_tree::ptree pt2;
		boost::property_tree::read_json(json_stream2, pt2);
		peer_manager.updateClientTree(id, pt2);
	}
}

void HTTPSClientPool::startPolling() {
	running.store(true);

	for (const auto& clientPair : clients) {
		auto server = clientPair.first.first;
		auto port = clientPair.first.second;
		LogWrite(PEERING__DEBUG, 5, "Peering", "%s: startPolling for %s:%s.", __FUNCTION__, server.c_str(), port.c_str());
		try {
			std::async(std::launch::async, [this, server, port]() {
				try {
					pollPeerHealth(server, port);
				}
				catch (const std::exception& e) {
					LogWrite(PEERING__DEBUG, 1, "Peering", "Exception in pollPeerHealth for %s:%s: %s", server.c_str(), port.c_str(), e.what());
				}
				catch (...) {
					LogWrite(PEERING__DEBUG, 1, "Peering", "Unknown exception in pollPeerHealth for %s:%s.", server.c_str(), port.c_str());
				}
				});
		}
		catch (const std::exception& e) {
			LogWrite(PEERING__DEBUG, 1, "Peering", "Failed to start async task for %s:%s: %s", server.c_str(), port.c_str(), e.what());
		}
		catch (...) {
			LogWrite(PEERING__DEBUG, 1, "Peering", "Unknown exception when starting async task for %s:%s.", server.c_str(), port.c_str());
		}
	}
}

void HTTPSClientPool::stopPolling() {
	running.store(false);
}

void HTTPSClientPool::sendPostRequestToPeerAsync(const std::string& peerId, const std::string& server, const std::string& port, const std::string& target, const std::string& payload) {
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		taskQueue.emplace([this, peerId, server, port, target, payload]() {
			std::shared_ptr<HTTPSClient> client = getClient(peerId);
			if (client) {
				std::string response = client->sendPostRequest(server, port, target, payload);

				boost::property_tree::ptree pt;

				try {
					std::istringstream responseStream(response);
					boost::property_tree::read_json(responseStream, pt);
				}
				catch (const boost::property_tree::json_parser_error& e) {
					LogWrite(PEERING__ERROR, 0, "Peering", "%s: JSON Parsing error for %s (%s:%s): %s.", __FUNCTION__, peerId.c_str(), server.c_str(), port.c_str(), e.what() ? e.what() : "??");
				}
				if (endpointHandlers.find(target) != endpointHandlers.end()) {
					endpointHandlers[target](pt);  // Call the corresponding handler
				}
				else {
					//std::cout << "No handler for endpoint: " << endpoint << std::endl;
				}
			}
			else {
				LogWrite(PEERING__ERROR, 0, "Peering", "%s: Client not found for %s (%s:%s).", __FUNCTION__, peerId.c_str(), server.c_str(), port.c_str());
			}
			});
	}
	condition.notify_one();
}

void HTTPSClientPool::workerFunction() {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			condition.wait(lock, [this] { return stop || !taskQueue.empty(); });
			if (stop && taskQueue.empty()) {
				return;
			}
			task = std::move(taskQueue.front());
			taskQueue.pop();
		}
		task();
	}
}