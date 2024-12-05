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
#include "../World.h"
#include "../WorldDatabase.h"
#include "../LoginServer.h"
#include "../LuaInterface.h"
#include "../Guilds/Guild.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

extern ZoneList zone_list;
extern World world;
extern LoginServer loginserver;
extern sint32 numclients;
extern WorldDatabase database;
extern ZoneList zone_list;
extern ZoneAuth zone_auth;
extern LuaInterface* lua_interface;
extern ConfigReader configReader;

extern MasterQuestList master_quest_list;
extern MasterSpellList master_spell_list;
extern MasterFactionList master_faction_list;
extern ClientList client_list;
extern GuildList guild_list;

PeerManager peer_manager;
HTTPSClientPool peer_https_pool;

void World::Web_worldhandle_status(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt;

	pt.put("web_status", "online");
	bool world_online = world.world_loaded;
	pt.put("world_status", world.world_loaded ? "online" : "offline");
	pt.put("world_uptime", (getCurrentTimestamp() - world.world_uptime));
	auto [days, hours, minutes, seconds] = convertTimestampDuration((getCurrentTimestamp() - world.world_uptime));
	std::string uptime_str("Days: " + std::to_string(days) + ", " + "Hours: " + std::to_string(hours) + ", " + "Minutes: " + std::to_string(minutes) + ", " + "Seconds: " + std::to_string(seconds));
	pt.put("world_uptime_string", uptime_str);
	pt.put("login_connected", loginserver.Connected() ? "connected" : "disconnected");
	pt.put("player_count", zone_list.GetZonesPlayersCount());
	pt.put("client_count", numclients);
	pt.put("zones_connected", zone_list.Count());
	pt.put("world_reloading", world.IsReloadingSubsystems() ? "yes" : "no");
	pt.put("peer_primary", net.is_primary);
	pt.put("peer_priority", net.GetPeerPriority());
	pt.put("peer_client_address", std::string(net.GetWorldAddress()));
	pt.put("peer_client_internal_address", std::string(net.GetInternalWorldAddress()));
	pt.put("peer_client_port", std::to_string(net.GetWorldPort()));

	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_clients(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	zone_list.PopulateClientList(res);
}

void ZoneList::PopulateClientList(http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree maintree;

	std::ostringstream oss;

	MClientList.lock();
	map<string, Client*>::iterator itr;
	for (itr = client_map.begin(); itr != client_map.end(); itr++) {
		if (itr->second) {
			Client* cur = (Client*)itr->second;
			boost::property_tree::ptree pt;
			pt.put("character_id", cur->GetCharacterID());
			pt.put("character_name", cur->GetPlayer() ? cur->GetPlayer()->GetName() : "");
			pt.put("subtitle", cur->GetPlayer() ? cur->GetPlayer()->appearance.sub_title : "");
			pt.put("class1", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_class1() : 0);
			pt.put("class2", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_class2() : 0);
			pt.put("class3", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_class3() : 0);
			pt.put("deity", cur->GetPlayer() ? cur->GetPlayer()->GetDeity() : 0);
			pt.put("tradeskill_class1", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_tradeskill_class1() : 0);
			pt.put("tradeskill_class2", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_tradeskill_class2() : 0);
			pt.put("tradeskill_class3", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_tradeskill_class3() : 0);
			pt.put("race", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_race() : 0);
			pt.put("level", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_level() : 0);
			pt.put("effective_level", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_effective_level() : 0);
			pt.put("tradeskill_level", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_tradeskill_level() : 0);
			pt.put("account_age", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_account_age_base() : 0);
			pt.put("account_id", cur->GetAccountID());
			pt.put("version", cur->GetVersion());
			pt.put("status", cur->GetAdminStatus());
			pt.put("guild_id", cur->GetPlayer()->GetGuild() != nullptr ? cur->GetPlayer()->GetGuild()->GetID() : 0);
			pt.put("flags", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_flags() : 0);
			pt.put("flags2", cur->GetPlayer() ? cur->GetPlayer()->GetInfoStruct()->get_flags2() : 0);
			pt.put("adventure_class", cur->GetPlayer() ? cur->GetPlayer()->GetAdventureClass() : 0);
			pt.put("tradeskill_class", cur->GetPlayer() ? cur->GetPlayer()->GetTradeskillClass() : 0);
			pt.put("is_zoning", (cur->IsZoning() || !cur->IsReadyForUpdates()));

			bool linkdead = cur->GetPlayer() ? (((cur->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_LINKDEAD) > 0)) : false;
			pt.put("is_linkdead", linkdead);
			pt.put("in_zone", cur->IsReadyForUpdates());
			pt.put("zone_id", (cur->GetPlayer() && cur->GetPlayer()->GetZone()) ? cur->GetPlayer()->GetZone()->GetZoneID() : 0);
			pt.put("instance_id", (cur->GetPlayer() && cur->GetPlayer()->GetZone()) ? cur->GetPlayer()->GetZone()->GetInstanceID() : 0);
			pt.put("zonename", (cur->GetPlayer() && cur->GetPlayer()->GetZone()) ? cur->GetPlayer()->GetZone()->GetZoneName() : "N/A");
			pt.put("zonedescription", (cur->GetPlayer() && cur->GetPlayer()->GetZone()) ? cur->GetPlayer()->GetZone()->GetZoneDescription() : "");

			GroupMemberInfo* gmi = cur->GetPlayer()->GetGroupMemberInfo();
			int32 group_id = 0;
			bool group_leader = false;
			if (gmi && gmi->group_id) {
				group_id = gmi->group_id;
				group_leader = gmi->leader;
			}
			pt.put("group_id", group_id);
			pt.put("group_leader", group_leader);
			maintree.push_back(std::make_pair("", pt));
		}
	}
	MClientList.unlock();

	boost::property_tree::ptree result;
	result.add_child("Clients", maintree);
	boost::property_tree::write_json(oss, result);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_setadminstatus(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);

	sint16 status = 0;
	std::string charname("");
	bool got_status_field = false;
	if (auto name = json_tree.get_optional<std::string>("character_name")) {
		charname = name.get();
	}
	if (auto new_status = json_tree.get_optional<sint16>("new_status")) {
		status = new_status.get();
		got_status_field = true;
	}

	sint32 success = 0;

	if (got_status_field && charname.size() > 0 && database.UpdateAdminStatus((char*)charname.c_str(), status)) {

		Client* target = zone_list.GetClientByCharName(charname.c_str());
		if (target) {
			target->SetAdminStatus(status);
			target->Message(CHANNEL_COLOR_YELLOW, "Your admin status has been set to %i.", status);
		}
		success = 1;
	}
	else if (!got_status_field || charname.size() < 1) {
		success = -1;
	}

	pt.put("success", success);

	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_reloadrules(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);

	database.LoadRuleSets(true);

	pt.put("success", 1);

	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_reloadcommand(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);

	int32 reload_command = 0, sub_command = 0;
	if (auto command = json_tree.get_optional<int32>("reload_command")) {
		reload_command = command.get();
	}
	if (auto subcommand = json_tree.get_optional<int32>("sub_command")) {
		sub_command = subcommand.get();
	}
	int32 success = 1;
	switch (reload_command) {
	case COMMAND_RELOADSTRUCTS: {
		world.SetReloadingSubsystem("Structs");
		configReader.ReloadStructs();
		world.RemoveReloadingSubSystem("Structs");
		break;
	}
	case COMMAND_RELOAD_QUESTS: {
		world.SetReloadingSubsystem("Quests");
		master_quest_list.Reload();
		client_list.ReloadQuests();
		zone_list.ReloadClientQuests();
		world.RemoveReloadingSubSystem("Quests");
		break;
	}
	case COMMAND_RELOAD_SPELLS: {
		if (sub_command == 1) { // npc only
			world.PurgeNPCSpells();
			database.LoadNPCSpells();
		}
		else {
			world.SetReloadingSubsystem("Spells");

			zone_list.DeleteSpellProcess();
			if (lua_interface)
				lua_interface->DestroySpells();
			master_spell_list.Reload();
			zone_list.LoadSpellProcess();
			world.RemoveReloadingSubSystem("Spells");
			world.PurgeNPCSpells();
			database.LoadNPCSpells();
		}
		break;
	}
	case COMMAND_RELOAD_ZONESCRIPTS: {
		world.SetReloadingSubsystem("ZoneScripts");
		world.ResetZoneScripts();
		database.LoadZoneScriptData();
		if (lua_interface)
			lua_interface->DestroyZoneScripts();
		world.RemoveReloadingSubSystem("ZoneScripts");
		break;
	}
	case COMMAND_RELOAD_FACTIONS: {
		world.SetReloadingSubsystem("Factions");
		master_faction_list.Clear();
		database.LoadFactionList();
		world.RemoveReloadingSubSystem("Factions");
		break;
	}
	case COMMAND_RELOAD_MAIL: {
		zone_list.ReloadMail();
		break;
	}
	case COMMAND_RELOAD_GUILDS: {
		world.ReloadGuilds();
		break;
	}
	case COMMAND_RELOAD_RULES: {
		database.LoadRuleSets(true);
		break;
	}
	case COMMAND_RELOAD_STARTABILITIES: {
		world.PurgeStartingLists();
		world.LoadStartingLists();
		break;
	}
	case COMMAND_RELOAD_VOICEOVERS: {
		world.PurgeVoiceOvers();
		world.LoadVoiceOvers();
		break;
	}
	case COMMAND_RELOADSPAWNSCRIPTS: {
		if (lua_interface)
			lua_interface->SetLuaSystemReloading(true);
		world.ResetSpawnScripts();
		database.LoadSpawnScriptData();
		if (lua_interface) {
			lua_interface->DestroySpawnScripts();
			lua_interface->SetLuaSystemReloading(false);
		}
		break;
	}
	case COMMAND_RELOADREGIONSCRIPTS: {
		if (lua_interface) {
			lua_interface->DestroyRegionScripts();
		}
		break;
	}
	case COMMAND_RELOADLUASYSTEM: {

		world.SetReloadingSubsystem("LuaSystem");

		if (lua_interface) {
			lua_interface->SetLuaSystemReloading(true);
		}

		zone_list.DeleteSpellProcess();
		if (lua_interface)
			lua_interface->DestroySpells();
		master_spell_list.Reload();
		zone_list.LoadSpellProcess();
		if (lua_interface) {
			map<Client*, int32> debug_clients = lua_interface->GetDebugClients();
			map<Client*, int32>::iterator itr;
			for (itr = debug_clients.begin(); itr != debug_clients.end(); itr++) {
				if (lua_interface)
					lua_interface->UpdateDebugClients(itr->first);
			}
		}

		world.ResetSpawnScripts();
		database.LoadSpawnScriptData();

		world.ResetZoneScripts();
		database.LoadZoneScriptData();

		if (lua_interface) {
			lua_interface->DestroySpawnScripts();
			lua_interface->DestroyRegionScripts();
			lua_interface->DestroyQuests();
			lua_interface->DestroyItemScripts();
			lua_interface->DestroyZoneScripts();
		}

		int32 quest_count = database.LoadQuests();

		int32 spell_count = 0;

		if (lua_interface) {
			spell_count = database.LoadSpellScriptData();
			lua_interface->SetLuaSystemReloading(false);
		}

		world.RemoveReloadingSubSystem("LuaSystem");

		break;
	}
	default: {
		success = 0;
		break;
	}
	}

	pt.put("success", success);

	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_addpeer(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);

	int16 client_port = 0;
	std::string client_addr("");
	std::string client_internal_addr("");

	int16 web_port = 0;
	std::string web_addr("");
	bool got_port = false;
	bool got_addr = false;
	if (auto client_address = json_tree.get_optional<std::string>("client_address")) {
		client_addr = client_address.get();
		got_addr = true;
	}

	if (auto client_internal_address = json_tree.get_optional<std::string>("client_internal_address")) {
		client_internal_addr = client_internal_address.get();
	}

	if (auto json_port = json_tree.get_optional<int16>("client_port")) {
		client_port = json_port.get();
		got_port = true;
	}

	if (auto world_address = json_tree.get_optional<std::string>("web_address")) {
		web_addr = world_address.get();
	}
	else if (got_addr)
		got_addr = false;

	if (auto port = json_tree.get_optional<int16>("web_port")) {
		web_port = port.get();
	}
	else if (got_port)
		got_port = false;

	sint32 success = 0;
	std::string peerName = peer_manager.isPeer(web_addr, web_port);
	if (got_port && got_addr && peerName.size() < 1) {
		std::string name = peer_manager.assignUniqueNameForSecondary("eq2emu_", client_addr, client_internal_addr, client_port, web_addr, web_port);
		peer_https_pool.addPeerClient(name, web_addr, std::to_string(web_port), "/addpeer");
		pt.put("assigned_peer_name", name.c_str());
		pt.put("peer_client_address", std::string(net.GetWorldAddress()));
		pt.put("peer_client_internal_address", std::string(net.GetInternalWorldAddress()));
		pt.put("peer_client_port", std::to_string(net.GetWorldPort()));
		pt.put("peer_web_address", std::string(net.GetWebWorldAddress()));
		pt.put("peer_web_port", std::to_string(net.GetWebWorldPort()));
		if (!peer_manager.hasPeers() && !net.is_primary) {
			net.SetPrimary();
		}
		pt.put("peer_primary", net.is_primary);
	}
	else {
		peer_manager.updatePeer(web_addr, web_port, client_addr, client_internal_addr, client_port, false);
		pt.put("assigned_peer_name", peerName);
		pt.put("peer_client_address", std::string(net.GetWorldAddress()));
		pt.put("peer_client_internal_address", std::string(net.GetInternalWorldAddress()));
		pt.put("peer_client_port", std::to_string(net.GetWorldPort()));
		pt.put("peer_web_address", std::string(net.GetWebWorldAddress()));
		pt.put("peer_web_port", std::to_string(net.GetWebWorldPort()));
		pt.put("peer_primary", net.is_primary);
	}

	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_zones(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	zone_list.PopulateZoneList(res);
}

void ZoneList::PopulateZoneList(http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree maintree;

	std::ostringstream oss;
	list<ZoneServer*>::iterator zone_iter;
	ZoneServer* tmp = 0;
	MZoneList.readlock(__FUNCTION__, __LINE__);
	int zonesListed = 0;
	for (zone_iter = zlist.begin(); zone_iter != zlist.end(); zone_iter++) {
		tmp = *zone_iter;
		boost::property_tree::ptree pt;
		pt.put("zone_name", tmp->GetZoneName());
		pt.put("zone_file_name", tmp->GetZoneFile());
		pt.put("zone_id", tmp->GetZoneID());
		pt.put("instance_id", tmp->GetInstanceID());
		pt.put("shutting_down", tmp->isZoneShuttingDown());
		pt.put("instance_zone", tmp->IsInstanceZone());
		pt.put("num_players", tmp->NumPlayers());
		pt.put("city_zone", tmp->IsCityZone());
		pt.put("safe_x", tmp->GetSafeX());
		pt.put("safe_y", tmp->GetSafeY());
		pt.put("safe_z", tmp->GetSafeZ());
		pt.put("safe_heading", tmp->GetSafeHeading());
		pt.put("lock_state", tmp->GetZoneLockState());
		pt.put("min_status", tmp->GetMinimumStatus());
		pt.put("min_level", tmp->GetMinimumLevel());
		pt.put("max_level", tmp->GetMaximumLevel());
		pt.put("min_version", tmp->GetMinimumVersion());
		pt.put("default_lockout_time", tmp->GetDefaultLockoutTime());
		pt.put("default_reenter_time", tmp->GetDefaultReenterTime());
		pt.put("instance_type", (int8)tmp->GetInstanceType());
		pt.put("always_loaded", tmp->AlwaysLoaded());
		zonesListed++;
		maintree.push_back(std::make_pair("", pt));
	}
	MZoneList.releasereadlock(__FUNCTION__, __LINE__);

	boost::property_tree::ptree result;
	result.add_child("Zones", maintree);
	boost::property_tree::write_json(oss, result);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_addcharauth(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);

	int32 account_id = 0;
	int32 key = 0;
	int32 zoneId, instanceId;
	bool firstLogin = false;

	std::string clientIP("");
	std::string charName(""), zoneName("");
	int32 charID = 0;
	int32 worldID = 0, fromID = 0;
	if (auto acct_id = json_tree.get_optional<int32>("account_id")) {
		account_id = acct_id.get();
	}

	if (auto char_name = json_tree.get_optional<std::string>("character_name")) {
		charName = char_name.get();
	}

	if (auto zone_name = json_tree.get_optional<std::string>("zone_name")) {
		zoneName = zone_name.get();
	}

	if (auto login_key = json_tree.get_optional<int32>("login_key")) {
		key = login_key.get();
	}

	if (auto zone_id = json_tree.get_optional<int32>("zone_id")) {
		zoneId = zone_id.get();
	}

	if (auto instance_id = json_tree.get_optional<int32>("instance_id")) {
		instanceId = instance_id.get();
	}

	if (auto first_login = json_tree.get_optional<bool>("first_login")) {
		firstLogin = first_login.get();
	}

	if (auto clientip = json_tree.get_optional<std::string>("client_ip")) {
		clientIP = clientip.get();
	}
	if (auto character_id = json_tree.get_optional<int32>("character_id")) {
		charID = character_id.get();
	}
	if (auto world_id = json_tree.get_optional<int32>("world_id")) {
		worldID = world_id.get();
	}
	if (auto from_id = json_tree.get_optional<int32>("from_id")) {
		fromID = from_id.get();
	}

	sint32 success = 0;

	ZoneChangeDetails details;
	if (instanceId || zoneId || zoneName.length() > 0) {
		if (!instanceId) {
			if ((zone_list.GetZone(&details, zoneId, zoneName, firstLogin, false, true, false)))
				success = 1;
		}
		else {
			if ((zone_list.GetZoneByInstance(&details, instanceId, zoneId, firstLogin, false, true, false)))
				success = 1;
		}
	}
	if (!success) {
		// failed to find zone requested by peer
	}

	if (success && account_id && key && charName.length() > 0) {
		ZoneAuthRequest* zar = new ZoneAuthRequest(account_id, (char*)charName.c_str(), key);
		zar->setFirstLogin(firstLogin);
		zone_auth.AddAuth(zar);
		success = 1;
	}
	else {
		success = 0;
	}

	pt.put("character_name", charName);
	pt.put("account_id", account_id);
	pt.put("zone_name", zoneName);
	pt.put("zone_id", zoneId);
	pt.put("instance_id", instanceId);
	pt.put("first_login", firstLogin);
	pt.put("peer_client_address", std::string(net.GetWorldAddress()));
	pt.put("peer_client_internal_address", std::string(net.GetInternalWorldAddress()));
	pt.put("peer_client_port", std::to_string(net.GetWorldPort()));
	pt.put("client_ip", clientIP);
	pt.put("character_id", std::to_string(charID));
	pt.put("login_key", std::to_string(key));
	pt.put("world_id", std::to_string(worldID));
	pt.put("from_id", std::to_string(fromID));
	pt.put("success", success);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_startzone(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);

	int32 instanceId = 0;
	int32 zoneId = 0;
	std::string zoneName("");
	bool alwaysLoaded = false;
	int32 minLevel = 0, maxLevel = 0, avgLevel = 0, firstLevel = 0;
	if (auto inst_id = json_tree.get_optional<int32>("instance_id")) {
		instanceId = inst_id.get();
	}

	if (auto zone = json_tree.get_optional<std::string>("zone_name")) {
		zoneName = zone.get();
	}

	if (auto zone_id = json_tree.get_optional<int32>("zone_id")) {
		zoneId = zone_id.get();
	}

	if (auto always_loaded = json_tree.get_optional<bool>("always_loaded")) {
		alwaysLoaded = always_loaded.get();
	}
	
	if (auto level = json_tree.get_optional<int32>("min_level")) {
		minLevel = level.get();
	}
	
	if (auto level = json_tree.get_optional<int32>("max_level")) {
		maxLevel = level.get();
	}
	
	if (auto level = json_tree.get_optional<int32>("avg_level")) {
		avgLevel = level.get();
	}
	
	if (auto level = json_tree.get_optional<int32>("first_level")) {
		firstLevel = level.get();
	}

	sint32 success = 0;
	ZoneChangeDetails details;
	if (instanceId || zoneId || zoneName.length() > 0) {
		if (!instanceId) {
			if ((zone_list.GetZone(&details, zoneId, zoneName, true, false, false, false, false, alwaysLoaded)))
				success = 1;
		}
		else {
			if ((zone_list.GetZoneByInstance(&details, instanceId, zoneId, true, false, false, false, minLevel, maxLevel, avgLevel, firstLevel)))
				success = 1;
		}
	}

	pt.put("success", success);
	pt.put("peer_web_address", net.GetWebWorldAddress());
	pt.put("peer_web_port", net.GetWebWorldPort());
	if (success) {
		pt.put("instance_id", details.instanceId);
		pt.put("zone_id", details.zoneId);
		pt.put("zone_name", details.zoneName);
	}
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_sendglobalmessage(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	int32 success = 0;
	int8 language = 0;
	int16 in_channel = 0;
	std::string toName(""), fromName(""), msg("");
	int32 group_id = 0;
	int32 guild_id = 0;
	if (auto name = json_tree.get_optional<std::string>("to_name")) {
		toName = name.get();
	}
	if (auto name = json_tree.get_optional<std::string>("from_name")) {
		fromName = name.get();
	}
	if (auto message = json_tree.get_optional<std::string>("message")) {
		msg = message.get();
	}
	if (auto from_language = json_tree.get_optional<int8>("from_language")) {
		language = from_language.get();
	}
	if (auto channel = json_tree.get_optional<int16>("channel")) {
		in_channel = channel.get();
	}
	if (auto group = json_tree.get_optional<int32>("group_id")) {
		group_id = group.get();
	}
	if (auto guildID = json_tree.get_optional<int32>("guild_id")) {
		guild_id = guildID.get();
	}

	Client* find_client = zone_list.GetClientByCharName(toName.c_str());
	if (find_client && find_client->GetPlayer()->IsIgnored(fromName.c_str()))
		success = 0;
	else {
		switch (in_channel) {
		case CHANNEL_PRIVATE_TELL: {
			if (find_client && find_client->GetPlayer()) {
				success = 1;
				toName = std::string(find_client->GetPlayer()->GetName());
				find_client->HandleTellMessage(fromName.c_str(), msg.c_str(), toName.c_str(), language);
				if (find_client->GetPlayer()->get_character_flag(CF_AFK)) {
					find_client->HandleTellMessage(toName.c_str(), find_client->GetPlayer()->GetAwayMessage().c_str(), fromName.c_str(), find_client->GetPlayer()->GetCurrentLanguage());
					pt.put("away_message", find_client->GetPlayer()->GetAwayMessage());
					pt.put("away_language", find_client->GetPlayer()->GetCurrentLanguage());
				}
			}
			break;
		}
		case CHANNEL_GROUP_SAY:
		case CHANNEL_RAID_SAY: {
			if (group_id) {
				success = 1;
				if (fromName.size() > 0) {
					world.GetGroupManager()->GroupChatMessage(group_id, fromName, language, msg.c_str(), in_channel);
				}
				else {
					world.GetGroupManager()->GroupMessage(group_id, msg.c_str());
				}
			}
			break;
		}
		case CHANNEL_OUT_OF_CHARACTER: {
			success = 1;
			zone_list.SendZoneWideChannelMessage(fromName, "", in_channel, msg.c_str(), 0, "", language);
			break;
		}
		case CHANNEL_GUILD_SAY: {
			if (!guild_id)
				break;

			Guild* guild = guild_list.GetGuild(guild_id);
			if (guild) {
				guild->HandleGuildSay(fromName, msg.c_str(), language);
			}
			break;
		}
		case CHANNEL_OFFICER_SAY: {
			if (!guild_id)
				break;

			Guild* guild = guild_list.GetGuild(guild_id);
			if (guild) {
				guild->HandleGuildSay(fromName, msg.c_str(), language);
			}
			break;
		}
		case CHANNEL_GUILD_EVENT: {
			if (!guild_id)
				break;

			Guild* guild = guild_list.GetGuild(guild_id);
			if (guild) {
				guild->SendMessageToGuild(CHANNEL_GUILD_EVENT, msg.c_str());
			}
			break;
		}
		case CHANNEL_GUILD_CHAT: {

			Guild* guild = guild_list.GetGuild(guild_id);
			if (guild) {
				guild->SendGuildChatMessage(msg.c_str());
			}
			break;
		}
		}
	}

	pt.put("success", success);
	pt.put("from_name", fromName);
	pt.put("to_name", toName);
	pt.put("message", msg);
	pt.put("from_language", language);
	pt.put("channel", in_channel);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}


void World::Web_worldhandle_newgroup(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);

	GroupOptions options;
	int32 group_id = 0;
	bool is_client = false;
	std::string leader(""), member("");
	std::string web_address("");
	int16 web_port = 0;
	int32 success = 0;
	bool is_update = false;

	if (auto address = json_tree.get_optional<std::string>("peer_web_address")) {
		web_address = address.get();
	}
	if (auto webport = json_tree.get_optional<int16>("peer_web_port")) {
		web_port = webport.get();
	}
	if (auto leader_name = json_tree.get_optional<std::string>("leader_name")) {
		leader = leader_name.get();
	}
	if (auto member_name = json_tree.get_optional<std::string>("member_name")) {
		member = member_name.get();
	}
	if (auto isclient = json_tree.get_optional<bool>("is_client")) {
		is_client = isclient.get();
	}
	if (auto lootmethod = json_tree.get_optional<int8>("loot_method")) {
		options.loot_method = lootmethod.get();
	}
	if (auto itemrarity = json_tree.get_optional<int8>("loot_item_rarity")) {
		options.loot_items_rarity = itemrarity.get();
	}
	if (auto autosplit = json_tree.get_optional<int8>("auto_split")) {
		options.auto_split = autosplit.get();
	}
	if (auto defaultyell = json_tree.get_optional<int8>("default_yell")) {
		options.default_yell = defaultyell.get();
	}
	if (auto grouplockmethod = json_tree.get_optional<int8>("group_lock_method")) {
		options.group_lock_method = grouplockmethod.get();
	}
	if (auto groupautolock = json_tree.get_optional<int8>("group_auto_lock")) {
		options.group_autolock = groupautolock.get();
	}
	if (auto soloautolock = json_tree.get_optional<int8>("solo_auto_lock")) {
		options.solo_autolock = soloautolock.get();
	}
	if (auto autoloot = json_tree.get_optional<int8>("auto_loot_method")) {
		options.auto_loot_method = autoloot.get();
	}
	if (auto lastlootindex = json_tree.get_optional<int8>("last_looted_index")) {
		options.last_looted_index = lastlootindex.get();
	}
	if (auto groupid = json_tree.get_optional<int32>("group_id")) {
		group_id = groupid.get();
	}
	std::vector<int32> raidGroups;
	for (int i = 0; i < 4; i++) {
		std::string fieldName("group_id_");
		fieldName.append(std::to_string(i));
		if (auto raid_group_id = json_tree.get_optional<int32>(fieldName)) {
			int32 group_id = raid_group_id.get();
			if (group_id) {
				raidGroups.push_back(group_id);
			}
		}
	}

	if (auto isupdate = json_tree.get_optional<bool>("is_update")) {
		is_update = isupdate.get();
	}

	if (is_update) {
		std::vector<int32>::iterator group_itr;

		std::vector<int32> emptyGroup;
		bool self = false;
		if (raidGroups.size() < 1) {
			raidGroups.push_back(group_id);
			self = true;
		}

		for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
			world.GetGroupManager()->SetGroupOptions((*group_itr), &options);
			if (self) {
				world.GetGroupManager()->ClearGroupRaid((*group_itr));
				world.GetGroupManager()->SendGroupUpdate((*group_itr), nullptr, true);
			}
			else {
				world.GetGroupManager()->ReplaceRaidGroups((*group_itr), &raidGroups);
			}
		}
		success = 1;
	}
	else if (net.is_primary) {
		group_id = world.GetGroupManager()->NewGroup(nullptr, &options);
		peer_manager.sendPeersNewGroupRequest(web_address, web_port, group_id, leader, member, &options, "", &raidGroups);
		success = 1;
	}
	else if (group_id) {
		int32 result = world.GetGroupManager()->NewGroup(nullptr, &options, group_id);
		if (result) {
			success = 1;
		}
	}

	pt.put("success", success);
	pt.put("group_id", group_id);
	pt.put("leader_name", leader);
	pt.put("member_name", member);
	pt.put("loot_method", options.loot_method);
	pt.put("loot_items_rarity", options.loot_items_rarity);
	pt.put("auto_split", options.auto_split);
	pt.put("default_yell", options.default_yell);
	pt.put("group_lock_method", options.group_lock_method);
	pt.put("group_autolock", options.group_autolock);
	pt.put("solo_autolock", options.solo_autolock);
	pt.put("auto_loot_method", options.auto_loot_method);
	pt.put("last_looted_index", options.last_looted_index);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}


void World::Web_worldhandle_addgroupmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	bool is_update = false;
	int32 group_id = 0;
	GroupMemberInfo info;
	if (auto member_name = json_tree.get_optional<std::string>("member_name")) {
		info.name = member_name.get();
	}
	if (auto isclient = json_tree.get_optional<bool>("is_client")) {
		info.is_client = isclient.get();
	}
	if (auto groupid = json_tree.get_optional<int32>("group_id")) {
		group_id = groupid.get();
	}
	if (auto zone = json_tree.get_optional<std::string>("zone")) {
		info.zone = zone.get();
	}

	if (auto hp = json_tree.get_optional<sint32>("current_hp")) {
		info.hp_current = hp.get();
	}
	if (auto hp = json_tree.get_optional<sint32>("max_hp")) {
		info.hp_max = hp.get();
	}

	if (auto power = json_tree.get_optional<sint32>("current_power")) {
		info.power_current = power.get();
	}
	if (auto power = json_tree.get_optional<sint32>("max_power")) {
		info.power_max = power.get();
	}


	if (auto level = json_tree.get_optional<int16>("level_current")) {
		info.level_current = level.get();
	}
	if (auto level = json_tree.get_optional<int16>("level_max")) {
		info.level_max = level.get();
	}

	if (auto race = json_tree.get_optional<int8>("race_id")) {
		info.race_id = race.get();
	}
	if (auto class_ = json_tree.get_optional<int8>("class_id")) {
		info.class_id = class_.get();
	}

	if (auto isleader = json_tree.get_optional<bool>("is_leader")) {
		info.leader = isleader.get();
	}

	if (auto isupdate = json_tree.get_optional<bool>("is_update")) {
		is_update = isupdate.get();
	}

	if (auto mentor_target = json_tree.get_optional<int32>("mentor_target_char_id")) {
		info.mentor_target_char_id = mentor_target.get();
	}

	if (auto zoneID = json_tree.get_optional<int32>("zone_id")) {
		info.zone_id = zoneID.get();
	}

	if (auto instanceID = json_tree.get_optional<int32>("instance_id")) {
		info.zone_id = instanceID.get();
	}


	if (auto clientPeerAddr = json_tree.get_optional<std::string>("client_peer_address")) {
		info.client_peer_address = clientPeerAddr.get();
	}

	if (auto clientPeerPort = json_tree.get_optional<int16>("client_peer_port")) {
		info.client_peer_port = clientPeerPort.get();
	}

	if (auto raidLooter = json_tree.get_optional<bool>("is_raid_looter")) {
		info.is_raid_looter = raidLooter.get();
	}
	else {
		info.is_raid_looter = false;
	}

	bool success = false;
	if (is_update) {
		world.GetGroupManager()->UpdateGroupMemberInfoFromPeer(group_id, info.name, info.is_client, &info);
		world.GetGroupManager()->SendGroupUpdate(group_id);
		success = true;
	}
	else {
		success = world.GetGroupManager()->AddGroupMemberFromPeer(group_id, &info);
	}

	pt.put("success", success);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}


void World::Web_worldhandle_removegroupmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	int32 group_id = 0;
	int32 char_id = 0;
	std::string name("");
	bool is_client = false;
	if (auto member_name = json_tree.get_optional<std::string>("member_name")) {
		name = member_name.get();
	}
	if (auto isclient = json_tree.get_optional<bool>("is_client")) {
		is_client = isclient.get();
	}
	if (auto groupid = json_tree.get_optional<int32>("group_id")) {
		group_id = groupid.get();
	}
	if (auto charid = json_tree.get_optional<int32>("character_id")) {
		char_id = charid.get();
	}

	bool success = world.GetGroupManager()->RemoveGroupMember(group_id, name, is_client, char_id);
	pt.put("success", success);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_disbandgroup(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	int32 group_id = 0;
	if (auto groupid = json_tree.get_optional<int32>("group_id")) {
		group_id = groupid.get();
	}

	world.GetGroupManager()->RemoveGroup(group_id);
	pt.put("success", true);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_createguild(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	std::string newGuildName(""), leaderName("");
	int32 guildID = 0;
	if (auto name = json_tree.get_optional<std::string>("guild_name")) {
		newGuildName = name.get();
	}

	if (auto name = json_tree.get_optional<std::string>("leader_name")) {
		leaderName = name.get();
	}

	if (auto guild_id = json_tree.get_optional<int32>("guild_id")) {
		guildID = guild_id.get();
	}



	bool success = false;
	if (net.is_primary) {
		if (newGuildName.size() > 0) {
			guildID = world.CreateGuild(newGuildName.c_str());
			if (guildID) {
				peer_manager.sendPeersCreateGuild(guildID);
				success = true;
			}
		}
	}
	else if (guildID) {
		database.LoadGuild(guildID);
		success = true;
	}

	pt.put("success", success);
	pt.put("guild_id", guildID);
	pt.put("guild_name", newGuildName);
	pt.put("leader_name", leaderName);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_addguildmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	std::string invitedBy("");
	int32 guildID = 0, charID = 0, joinTimestamp = 0;
	int8 guildRank = 0;
	if (auto character_id = json_tree.get_optional<int32>("character_id")) {
		charID = character_id.get();
	}
	if (auto name = json_tree.get_optional<std::string>("invited_by")) {
		invitedBy = name.get();
	}
	if (auto guild_id = json_tree.get_optional<int32>("guild_id")) {
		guildID = guild_id.get();
	}
	if (auto join_timestamp = json_tree.get_optional<int8>("join_timestamp")) {
		joinTimestamp = join_timestamp.get();
	}

	if (auto rank = json_tree.get_optional<int8>("rank")) {
		guildRank = rank.get();
	}

	bool success = false;
	if (guildID) {
		Guild* guild = guild_list.GetGuild(guildID);
		if (guild) {
			guild->AddNewGuildMember(charID, invitedBy.c_str(), joinTimestamp, guildRank);
			success = true;
		}
	}
	pt.put("success", success);
	pt.put("guild_id", guildID);
	pt.put("invited_by", invitedBy);
	pt.put("character_id", charID);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_removeguildmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	std::string removedBy("");
	int32 guildID = 0, charID = 0;
	if (auto character_id = json_tree.get_optional<int32>("character_id")) {
		charID = character_id.get();
	}
	if (auto name = json_tree.get_optional<std::string>("removed_by")) {
		removedBy = name.get();
	}
	if (auto guild_id = json_tree.get_optional<int32>("guild_id")) {
		guildID = guild_id.get();
	}
	bool success = false;
	if (guildID) {
		Guild* guild = guild_list.GetGuild(guildID);
		if (guild) {
			guild->RemoveGuildMember(charID, true);
			success = true;
		}
	}
	pt.put("success", success);
	pt.put("guild_id", guildID);
	pt.put("removed_by", removedBy);
	pt.put("character_id", charID);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_setguildpermission(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	int32 guildID = 0;
	int8 rank = 0, permission = 0, value_ = 0;
	if (auto guild_id = json_tree.get_optional<int32>("guild_id")) {
		guildID = guild_id.get();
	}
	if (auto in_rank = json_tree.get_optional<int8>("rank")) {
		rank = in_rank.get();
	}
	if (auto in_permission = json_tree.get_optional<int8>("permission")) {
		permission = in_permission.get();
	}
	if (auto in_value = json_tree.get_optional<int8>("value")) {
		value_ = in_value.get();
	}
	bool success = false;
	if (guildID) {
		Guild* guild = guild_list.GetGuild(guildID);
		if (guild) {
			guild->SetPermission(rank, permission, value_, true, false);
			success = true;
		}
	}
	pt.put("success", success);
	pt.put("guild_id", guildID);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}

void World::Web_worldhandle_setguildeventfilter(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	res.set(http::field::content_type, "application/json; charset=utf-8");
	boost::property_tree::ptree pt, json_tree;

	std::istringstream json_stream(req.body());
	boost::property_tree::read_json(json_stream, json_tree);
	int32 guildID = 0;
	int8 eventID = 0, category = 0, value_ = 0;
	if (auto guild_id = json_tree.get_optional<int32>("guild_id")) {
		guildID = guild_id.get();
	}
	if (auto event_id = json_tree.get_optional<int8>("event_id")) {
		eventID = event_id.get();
	}
	if (auto in_category = json_tree.get_optional<int8>("category")) {
		category = in_category.get();
	}
	if (auto in_value = json_tree.get_optional<int8>("value")) {
		value_ = in_value.get();
	}
	bool success = false;
	if (guildID) {
		Guild* guild = guild_list.GetGuild(guildID);
		if (guild) {
			guild->SetEventFilter(eventID, category, value_, true, false);
			success = true;
		}
	}
	pt.put("success", success);
	pt.put("guild_id", guildID);
	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt);
	std::string json = oss.str();
	res.body() = json;
	res.prepare_payload();
}
