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

#include "PlayerGroups.h"
#include "../common/Log.h"
#include "World.h"
#include "Spells.h"
#include "LuaInterface.h"
#include "Bots/Bot.h"
#include "SpellProcess.h"
#include "Rules/Rules.h"
#include "Web/PeerManager.h"
#include "WorldDatabase.h"

extern ConfigReader configReader;
extern ZoneList	zone_list;
extern RuleManager rule_manager;
extern PeerManager peer_manager;
extern WorldDatabase database;
extern LuaInterface* lua_interface;
/******************************************************** PlayerGroup ********************************************************/

PlayerGroup::PlayerGroup(int32 id) {
	m_id = id;
	MGroupMembers.SetName("MGroupMembers");
	SetDefaultGroupOptions();
}

PlayerGroup::~PlayerGroup() {
	Disband();
}

bool PlayerGroup::AddMember(Entity* member, bool is_leader) {
	// Check to make sure the entity we are adding is valid
	if (!member) {
		LogWrite(GROUP__ERROR, 0, "Group", "New member is null");
		return false;
	}

	// Make sure entity we are adding isn't already in a group by checking if it has a GroupMemberInfo pointer
	if (member->GetGroupMemberInfo()) {
		LogWrite(GROUP__ERROR, 0, "Group", "New member (%s) already has a group", member->GetName());
		return false;
	}

	// Create a new GroupMemberInfo and assign it to the new member
	GroupMemberInfo* gmi = new GroupMemberInfo;
	gmi->group_id = m_id;
	gmi->member = member;
	gmi->leader = is_leader;
	if (member->IsPlayer()) {
		gmi->is_client = true;
		gmi->client = ((Player*)member)->GetClient();
	}
	else {
		gmi->is_client = false;
		gmi->client = 0;
	}
	gmi->mentor_target_char_id = 0;

	if (member->GetZone()) {
		gmi->zone_id = member->GetZone()->GetZoneID();
		gmi->instance_id = member->GetZone()->GetInstanceID();
	}

	gmi->client_peer_address = std::string(net.GetWorldAddress());
	gmi->client_peer_port = net.GetWorldPort();

	gmi->is_raid_looter = false;

	member->SetGroupMemberInfo(gmi);
	member->group_id = gmi->group_id;
	MGroupMembers.writelock();
	m_members.push_back(gmi);
	member->UpdateGroupMemberInfo(true, true);
	MGroupMembers.releasewritelock();

	SendGroupUpdate();
	return true;
}

bool PlayerGroup::AddMemberFromPeer(std::string name, bool isleader, bool isclient, int8 class_id, sint32 hp_cur, sint32 hp_max, int16 level_cur, int16 level_max,
	sint32 power_cur, sint32 power_max, int8 race_id, std::string zonename, int32 mentor_target_char_id, int32 zone_id, int32 instance_id,
	std::string peer_client_address, int16 peer_client_port, bool is_raid_looter) {
	// Create a new GroupMemberInfo and assign it to the new member
	GroupMemberInfo* gmi = new GroupMemberInfo;
	gmi->group_id = m_id;
	gmi->member = nullptr;
	gmi->leader = isleader;
	gmi->is_client = isclient;
	gmi->client = 0;
	gmi->mentor_target_char_id = 0;

	gmi->class_id = class_id;
	gmi->hp_max = hp_max;
	gmi->hp_current = hp_cur;
	gmi->level_max = level_max;
	gmi->level_current = level_cur;
	gmi->name = name;
	gmi->power_current = power_cur;
	gmi->power_max = power_max;
	gmi->race_id = race_id;
	gmi->zone = zonename;
	gmi->zone_id = zone_id;
	gmi->instance_id = instance_id;

	gmi->mentor_target_char_id = mentor_target_char_id;

	gmi->client_peer_address = peer_client_address;
	gmi->client_peer_port = peer_client_port;

	gmi->is_raid_looter = is_raid_looter;
	MGroupMembers.writelock();
	m_members.push_back(gmi);
	MGroupMembers.releasewritelock();

	SendGroupUpdate();
	return true;
}

bool PlayerGroup::RemoveMember(Entity* member) {
	GroupMemberInfo* gmi = member->GetGroupMemberInfo();
	if (!gmi) {
		return false;
	}

	bool ret = false;

	bool selfInRaid = IsInRaidGroup(gmi->group_id, false);

	MGroupMembers.writelock();
	member->SetGroupMemberInfo(0);

	deque<GroupMemberInfo*>::iterator erase_itr = m_members.end();
	deque<GroupMemberInfo*>::iterator itr;
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		if (gmi == *itr)
			erase_itr = itr;

		if (member->IsPlayer() && (*itr)->mentor_target_char_id == ((Player*)member)->GetCharacterID() && (*itr)->client)
		{
			(*itr)->mentor_target_char_id = 0;
			(*itr)->client->GetPlayer()->EnableResetMentorship();
		}

		if ((*itr)->client) {
			(*itr)->client->GetPlayer()->SetCharSheetChanged(true);
			if (selfInRaid)
				(*itr)->client->GetPlayer()->SetRaidSheetChanged(true);
		}
	}
	if (erase_itr != m_members.end()) {
		ret = true;
		m_members.erase(erase_itr);
	}
	MGroupMembers.releasewritelock();

	member->SetGroupMemberInfo(nullptr);
	safe_delete(gmi);
	if (member->IsBot())
		((Bot*)member)->Camp();

	return ret;
}

bool PlayerGroup::RemoveMember(std::string name, bool is_client, int32 charID) {
	bool ret = false;
	GroupMemberInfo* gmi = nullptr;
	bool selfInRaid = IsInRaidGroup(GetID(), false);
	deque<GroupMemberInfo*>::iterator erase_itr = m_members.end();
	deque<GroupMemberInfo*>::iterator itr;
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		if ((*itr)->name == name && (*itr)->is_client == is_client) {
			gmi = (*itr);
			erase_itr = itr;
		}

		if (is_client && charID > 0 && (*itr)->mentor_target_char_id == charID && (*itr)->client)
		{
			(*itr)->mentor_target_char_id = 0;
			(*itr)->client->GetPlayer()->EnableResetMentorship();
		}

		if ((*itr)->client) {
			(*itr)->client->GetPlayer()->SetCharSheetChanged(true);
			if (selfInRaid)
				(*itr)->client->GetPlayer()->SetRaidSheetChanged(true);
		}
	}
	if (erase_itr != m_members.end()) {
		ret = true;
		m_members.erase(erase_itr);
	}
	MGroupMembers.releasewritelock();

	safe_delete(gmi);

	return ret;
}

void PlayerGroup::Disband() {
	m_raidgroups.clear();
	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.writelock();

	bool selfInRaid = IsInRaidGroup(GetID(), false);

	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		if ((*itr)->member) {
			(*itr)->member->SetGroupMemberInfo(0);
			if ((*itr)->member->IsBot())
				((Bot*)(*itr)->member)->Camp();
		}
		if ((*itr)->mentor_target_char_id && (*itr)->client)
		{
			(*itr)->mentor_target_char_id = 0;
			(*itr)->client->GetPlayer()->EnableResetMentorship();
		}

		if ((*itr)->client) {
			(*itr)->client->GetPlayer()->SetCharSheetChanged(true);
			if (selfInRaid)
				(*itr)->client->GetPlayer()->SetRaidSheetChanged(true);
		}

		safe_delete(*itr);
	}

	m_members.clear();
	MGroupMembers.releasewritelock();
}

void PlayerGroup::SendGroupUpdate(Client* exclude, bool forceRaidUpdate) {
	bool selfInRaid = IsInRaidGroup(GetID(), false);
	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* gmi = *itr;
		if (gmi->client && gmi->client != exclude && !gmi->client->IsZoning()) {
			gmi->client->GetPlayer()->SetCharSheetChanged(true);
			if (selfInRaid || forceRaidUpdate)
				gmi->client->GetPlayer()->SetRaidSheetChanged(true);
		}
	}
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
}

void PlayerGroup::SimpleGroupMessage(const char* message) {
	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* info = *itr;
		if (info->client)
			info->client->SimpleMessage(CHANNEL_GROUP_CHAT, message);
	}
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
}

void PlayerGroup::SendGroupMessage(int8 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	buffer[0] = 0;
	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* info = *itr;
		if (info->client)
			info->client->SimpleMessage(type, buffer);
	}
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
}

void PlayerGroup::GroupChatMessage(Spawn* from, int32 language, const char* message, int16 channel) {
	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* info = *itr;
		if (info && info->client && info->client->GetCurrentZone())
			info->client->GetCurrentZone()->HandleChatMessage(info->client, from, 0, channel, message, 0, 0, true, language);
	}
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
}

void PlayerGroup::GroupChatMessage(std::string fromName, int32 language, const char* message, int16 channel) {
	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* info = *itr;
		if (info && info->client && info->client->GetCurrentZone())
			info->client->GetCurrentZone()->HandleChatMessage(info->client, fromName, "", channel, message, 0, 0, language);
	}
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
}

bool PlayerGroup::MakeLeader(Entity* new_leader) {
	if (!new_leader || new_leader->GetGroupMemberInfo())
		return false;

	bool selfInRaid = IsInRaidGroup(GetID(), false);

	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	new_leader->GetGroupMemberInfo()->leader = true;
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* info = *itr;
		if (info && info != new_leader->GetGroupMemberInfo() && info->leader) {
			info->leader = false;
			peer_manager.sendPeersGroupMember(GetID(), info, true);
			break;
		}
		if ((*itr)->client) {
			(*itr)->client->GetPlayer()->SetCharSheetChanged(true);
			if (selfInRaid)
				(*itr)->client->GetPlayer()->SetRaidSheetChanged(true);
		}
	}
	peer_manager.sendPeersGroupMember(GetID(), new_leader->GetGroupMemberInfo(), true);
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);


	SendGroupUpdate();

	return true;
}

std::string PlayerGroup::GetLeaderName() {
	std::string name("");
	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* info = *itr;
		if (info->leader && info->name.size() > 0) {
			name = info->name;
			break;
		}
	}
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	return name;
}

bool PlayerGroup::ShareQuestWithGroup(Client* quest_sharer, Quest* quest) {
	if (!quest || !quest_sharer)
		return false;

	bool canShare = quest->CanShareQuestCriteria(quest_sharer);

	if (!canShare) {
		return false;
	}

	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* info = *itr;
		if (info && info->client && info->client->GetCurrentZone()) {
			if (quest_sharer != info->client && info->client->GetPlayer()->HasAnyQuest(quest->GetQuestID()) == 0) {
				info->client->AddPendingQuest(new Quest(quest));
				info->client->Message(CHANNEL_COLOR_YELLOW, "%s has shared the quest %s with you.", quest_sharer->GetPlayer()->GetName(), quest->GetName());
			}
		}
	}
	MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);

	return true;
}



/******************************************************** PlayerGroupManager ********************************************************/

PlayerGroupManager::PlayerGroupManager() {
	m_nextGroupID = 1;

	MPendingInvites.SetName("PlayerGroupManager::m_pendingInvites");
}

PlayerGroupManager::~PlayerGroupManager() {
	MPendingInvites.writelock(__FUNCTION__, __LINE__);
	m_pendingInvites.clear();
	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);

	std::unique_lock lock(MGroups);
	map<int32, PlayerGroup*>::iterator itr;
	for (itr = m_groups.begin(); itr != m_groups.end(); itr++)
		safe_delete(itr->second);

	m_groups.clear();
}

bool PlayerGroupManager::AddGroupMember(int32 group_id, Entity* member, bool is_leader) {
	std::shared_lock lock(MGroups);
	bool ret = false;

	if (m_groups.count(group_id) > 0) {
		PlayerGroup* group = m_groups[group_id];
		ret = group->AddMember(member, is_leader);
		peer_manager.sendPeersGroupMember(group_id, member->GetGroupMemberInfo());
	}

	return ret;
}

bool PlayerGroupManager::AddGroupMemberFromPeer(int32 group_id, GroupMemberInfo* info) {
	if (!info)
		return false;

	std::shared_lock lock(MGroups);
	bool ret = false;

	if (m_groups.count(group_id) > 0) {
		PlayerGroup* group = m_groups[group_id];
		ret = group->AddMemberFromPeer(info->name, info->leader, info->is_client, info->class_id, info->hp_current, info->hp_max, info->level_current, info->level_max,
			info->power_current, info->power_max, info->race_id, info->zone, info->mentor_target_char_id, info->zone_id, info->instance_id,
			info->client_peer_address, info->client_peer_port, info->is_raid_looter);
	}

	return ret;
}

bool PlayerGroupManager::RemoveGroupMember(int32 group_id, Entity* member) {
	if (!member)
		return false;

	bool ret = false;
	bool remove = false;
	Client* client = 0;
	if (member->GetGroupMemberInfo() && member->GetGroupMemberInfo()->mentor_target_char_id)
	{
		if (member->IsPlayer())
		{
			Player* tmpPlayer = (Player*)member;
			member->GetGroupMemberInfo()->mentor_target_char_id = 0;
			tmpPlayer->EnableResetMentorship();
		}
	}

	GroupLock(__FUNCTION__, __LINE__);

	if (m_groups.count(group_id) > 0) {
		PlayerGroup* group = m_groups[group_id];

		if (member->IsPlayer())
			client = member->GetGroupMemberInfo()->client;

		ret = group->RemoveMember(member);

		// If only 1 person left in the group set a flag to remove the group
		if (group->Size() == 1)
			remove = true;
	}

	ReleaseGroupLock(__FUNCTION__, __LINE__);

	if (client)
		RemoveGroupBuffs(group_id, client);

	peer_manager.sendPeersRemoveGroupMember(group_id, std::string(member->GetName()), (member->IsPlayer() ? ((Player*)member)->GetCharacterID() : 0), member->IsPlayer());

	// Call RemoveGroup outside the locks as it uses the same locks
	if (remove) {
		RemoveGroup(group_id);
		peer_manager.sendPeersDisbandGroup(group_id);
	}

	return ret;
}
bool PlayerGroupManager::RemoveGroupMember(int32 group_id, std::string name, bool is_client, int32 charID) {
	bool ret = false;
	bool remove = false;
	Client* client = 0;

	GroupLock(__FUNCTION__, __LINE__);

	if (m_groups.count(group_id) > 0) {
		PlayerGroup* group = m_groups[group_id];

		ret = group->RemoveMember(name, is_client, charID);

		// If only 1 person left in the group set a flag to remove the group
		if (group->Size() == 1)
			remove = true;
	}

	ReleaseGroupLock(__FUNCTION__, __LINE__);

	//if (client)
	//	RemoveGroupBuffs(group_id, client);

	// Call RemoveGroup outside the locks as it uses the same locks
	if (remove)
		RemoveGroup(group_id);
	else {
		GroupMessage(group_id, "%s has left the group.", name.c_str());
	}
	return ret;
}

int32 PlayerGroupManager::NewGroup(Entity* leader, GroupOptions* goptions, int32 override_group_id) {
	std::unique_lock lock(MGroups);
	int32 groupID = 0;

	// Highly doubt this will ever be needed but putting it in any way, basically bump the id and ensure
	// no active group is currently using this id, if we hit the max for an int32 then reset the id to 1
	if (!override_group_id) {
		do {
			groupID = peer_manager.getUniqueGroupId();
		} while (m_groups.count(groupID) > 0);
	}
	else if (override_group_id) {
		groupID = override_group_id;
		if (m_groups.count(groupID))
			return 0; // group already exists
	}

	// last resort if the unique group id is not working out
	while (m_groups.count(groupID) > 0) {
		// If m_nextGroupID is at its max then reset it to 1, else increment it
		if (groupID == 4294967295)
			groupID = 1;
		else
			groupID++;
	}

	// Create a new group with the valid ID we got from above
	PlayerGroup* new_group = new PlayerGroup(groupID);
	new_group->SetDefaultGroupOptions(goptions);

	// Add the new group to the list (need to do this first, AddMember needs ref to the PlayerGroup ptr -> UpdateGroupMemberInfo)
	m_groups[groupID] = new_group;

	return groupID;
}

void PlayerGroupManager::RemoveGroup(int32 group_id) {
	std::unique_lock lock(MGroups);

	// Check to see if the id is in the list
	if (m_groups.count(group_id) > 0) {
		// Get a pointer to the group
		PlayerGroup* group = m_groups[group_id];

		std::vector<int32> raidGroups;
		m_groups[group_id]->GetRaidGroups(&raidGroups);
		if (raidGroups.size() > 0) {
			std::vector<int32>::iterator group_itr;
			for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
				if (m_groups.count((*group_itr))) {
					m_groups[(*group_itr)]->ReplaceRaidGroups(&raidGroups);
				}
			}
		}
		m_groups[group_id]->ClearGroupRaid();
		// Erase the group from the list
		m_groups.erase(group_id);
		// Delete the group
		safe_delete(group);
	}
}

int8 PlayerGroupManager::Invite(Player* leader, Entity* member) {
	int8 ret = 255; // Should be changed, if it is not then we have an unknown error

	// Lock the pending invite list so we can work with it
	MPendingInvites.writelock(__FUNCTION__, __LINE__);

	if (!member || (member->IsBot() && ((Bot*)member)->IsImmediateCamp()))
		ret = 6; // failure, not a valid target
	else if (member->IsNPC() && (!member->IsBot() /*|| !member->IsMec()*/))
		ret = 6;
	else if (leader == member)
		ret = 5; // failure, can't invite yourself
	else if (member->GetGroupMemberInfo())
		ret = 1; // failure, member already in a group
	// Check to see if the target of the invite already has a pending invite
	else if (m_pendingInvites.count(member->GetName()) > 0)
		ret = 2; // Target already has an invite
	// Check to see if the player that invited is already in a group
	else if (leader->GetGroupMemberInfo()) {
		// Read lock the group list so we can get the size of the inviters group
		GroupLock(__FUNCTION__, __LINE__);
		int32 group_size = m_groups[leader->GetGroupMemberInfo()->group_id]->Size();
		ReleaseGroupLock(__FUNCTION__, __LINE__);

		// Check to see if the group is full
		if (m_groups[leader->GetGroupMemberInfo()->group_id]->Size() >= 6)
			ret = 3; // Group full
		// Group isn't full so add the member to the pending invite list
		else {
			m_pendingInvites[member->GetName()] = leader->GetName();
			ret = 0; // Success
		}
	}
	// Inviter is not in a group
	else {
		// Check to see if the inviter has a pending invite himself
		if (m_pendingInvites.count(leader->GetName()) > 0)
			ret = 4;	// inviter already has a pending group invite
		// No pending invites for the inviter add both the inviter and the target of the invite to the list
		else {
			m_pendingInvites[leader->GetName()] = leader->GetName();
			m_pendingInvites[member->GetName()] = leader->GetName();
			ret = 0; // success
		}
	}
	// Release the lock on pending invites
	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);

	bool group_existed = false;
	if (leader && leader->GetGroupMemberInfo()) {
		group_existed = true;
	}
	/* testing purposes only */
	if (ret == 0 && member->IsNPC()) {
		if (net.is_primary) {
			int32 group_id = 0;
			int8 result = AcceptInvite(member, &group_id, false);
			if (result == 0) {
				if (leader && leader->GetClient()) {
					if (group_existed) {
						group_id = leader->GetGroupMemberInfo()->group_id;
						GroupOptions options;
						leader->GetClient()->SetGroupOptionsReference(&options);
						peer_manager.sendPeersNewGroupRequest("", 0, group_id, std::string(leader->GetName()), std::string(member->GetName()), &options);
					}
					else {
						AddGroupMember(group_id, leader, true);
					}
					GroupMessage(group_id, "%s has joined the group.", member->GetName());
					AddGroupMember(group_id, member);
				}
			}
		}
		else {
			GroupOptions options;
			if (leader && leader->GetClient()) {
				GroupOptions options;
				leader->GetClient()->SetGroupOptionsReference(&options);
				peer_manager.sendPrimaryNewGroupRequest(std::string(leader->GetName()), std::string(member->GetName()), member->GetID(), &options);
			}
		}
	}

	return ret;
}

bool PlayerGroupManager::AddInvite(Player* leader, Entity* member) {
	bool ret = true;
	MPendingInvites.writelock(__FUNCTION__, __LINE__);
	if (leader == member)
		ret = false; // failure, can't invite yourself
	else if (member->GetGroupMemberInfo())
		ret = false;
	// Check to see if the target of the invite already has a pending invite
	else if (m_pendingInvites.count(member->GetName()) > 0)
		ret = false; // Target already has an invite

	if (leader->GetGroupMemberInfo()) {
		m_pendingInvites[member->GetName()] = leader->GetName();
	}
	else {
		m_pendingInvites[leader->GetName()] = leader->GetName();
		m_pendingInvites[member->GetName()] = leader->GetName();
	}
	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

int8 PlayerGroupManager::AcceptInvite(Entity* member, int32* group_override_id, bool auto_add_group) {
	int8 ret = 3; // default to unknown error
	int32 groupResultID = 0;
	MPendingInvites.writelock(__FUNCTION__, __LINE__);

	if (m_pendingInvites.count(member->GetName()) > 0) {
		string leader = m_pendingInvites[member->GetName()];
		Client* client_leader = zone_list.GetClientByCharName(leader);

		if (client_leader) {
			if (m_pendingInvites.count(leader) > 0) {
				if (!client_leader->GetPlayer()->GetGroupMemberInfo()) {
					GroupOptions goptions;
					goptions.loot_method = client_leader->GetPlayer()->GetInfoStruct()->get_group_loot_method();
					goptions.loot_items_rarity = client_leader->GetPlayer()->GetInfoStruct()->get_group_loot_items_rarity();
					goptions.auto_split = client_leader->GetPlayer()->GetInfoStruct()->get_group_auto_split();
					goptions.default_yell = client_leader->GetPlayer()->GetInfoStruct()->get_group_default_yell();
					goptions.group_autolock = client_leader->GetPlayer()->GetInfoStruct()->get_group_autolock();
					goptions.group_lock_method = client_leader->GetPlayer()->GetInfoStruct()->get_group_lock_method();
					goptions.solo_autolock = client_leader->GetPlayer()->GetInfoStruct()->get_group_solo_autolock();
					goptions.auto_loot_method = client_leader->GetPlayer()->GetInfoStruct()->get_group_auto_loot_method();
					groupResultID = NewGroup(client_leader->GetPlayer(), &goptions, (group_override_id != nullptr) ? (*group_override_id) : 0);
					if (group_override_id != nullptr && *group_override_id == 0) {
						*group_override_id = groupResultID;
					}
				}
				else {
					if (group_override_id != nullptr && *group_override_id == 0) {
						if (client_leader->GetPlayer()->GetGroupMemberInfo())
							*group_override_id = client_leader->GetPlayer()->GetGroupMemberInfo()->group_id;
					}
				}
				m_pendingInvites.erase(leader);
			}

			// Remove from invite list and add to the group
			if (m_pendingInvites.count(member->GetName())) {
				m_pendingInvites.erase(member->GetName());
			}
			int32 result_group_id = 0;
			if (*group_override_id && *group_override_id > 0) {
				result_group_id = *group_override_id;
			}
			else
				result_group_id = groupResultID;

			if (auto_add_group && result_group_id) {
				AddGroupMember(result_group_id, client_leader->GetPlayer());
				GroupMessage(result_group_id, "%s has joined the group.", member->GetName());
				AddGroupMember(result_group_id, member);
			}
			ret = 0; // success
		}
		else {
			// Was unable to find the leader, remove from the invite list
			if (m_pendingInvites.count(member->GetName())) {
				m_pendingInvites.erase(member->GetName());
			}
			ret = 2; // failure, can't find leader
		}
	}
	else
		ret = 1; // failure, no pending invite

	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

void PlayerGroupManager::DeclineInvite(Entity* member) {
	MPendingInvites.writelock(__FUNCTION__, __LINE__);

	if (m_pendingInvites.count(member->GetName()) > 0) {
		string leader = m_pendingInvites[member->GetName()];
		// send decline to leader
		m_pendingInvites.erase(member->GetName());
		if (m_pendingInvites.count(leader) > 0)
			m_pendingInvites.erase(leader);
	}

	if (m_raidPendingInvites.count(member->GetName()) > 0) {
		string leader = m_raidPendingInvites[member->GetName()];
		// send decline to leader
		m_raidPendingInvites.erase(member->GetName());
	}

	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);
}

bool PlayerGroupManager::IsGroupIDValid(int32 group_id) {
	std::shared_lock lock(MGroups);
	bool ret = false;
	ret = m_groups.count(group_id) > 0;
	return ret;
}

void PlayerGroupManager::SendGroupUpdate(int32 group_id, Client* exclude, bool forceRaidUpdate) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(group_id) > 0) {
		std::vector<int32> raidGroups;
		m_groups[group_id]->GetRaidGroups(&raidGroups);
		if (raidGroups.size() < 1)
			raidGroups.push_back(group_id);
		std::vector<int32>::iterator group_itr;
		for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
			if (m_groups.count((*group_itr))) {
				m_groups[(*group_itr)]->SendGroupUpdate(exclude, forceRaidUpdate);
			}
		}
	}
}

PlayerGroup* PlayerGroupManager::GetGroup(int32 group_id) {
	if (m_groups.count(group_id) > 0)
		return m_groups[group_id];

	return 0;
}

void PlayerGroupManager::ClearPendingInvite(Entity* member) {
	MPendingInvites.writelock(__FUNCTION__, __LINE__);

	if (m_pendingInvites.count(member->GetName()) > 0)
		m_pendingInvites.erase(member->GetName());

	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);
}

std::string PlayerGroupManager::HasPendingInvite(Entity* member) {
	std::string leader("");
	MPendingInvites.writelock(__FUNCTION__, __LINE__);

	if (m_pendingInvites.count(member->GetName()) > 0) {
		leader = m_pendingInvites[member->GetName()];
	}

	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);
	return leader;
}

void PlayerGroupManager::RemoveGroupBuffs(int32 group_id, Client* client) {
	SpellEffects* se = 0;
	Spell* spell = 0;
	LuaSpell* luaspell = 0;
	EQ2Packet* packet = 0;
	Entity* pet = 0;
	Player* player = 0;
	Entity* charmed_pet = 0;
	PlayerGroup* group = 0;

	MGroups.lock_shared();
	if (m_groups.count(group_id) > 0)
		group = m_groups[group_id];

	if (group && client) {
		/* first remove all spell effects this group member has on them from other group members */
		player = client->GetPlayer();
		bool recoup_lock = true;
		for (int i = 0; i < NUM_SPELL_EFFECTS; i++) {
			if (recoup_lock) {
				player->GetSpellEffectMutex()->readlock(__FUNCTION__, __LINE__);
				recoup_lock = false;
				se = player->GetSpellEffects();
			}
			if (se && se[i].spell_id != 0xFFFFFFFF) {
				//If the client is the caster, don't remove the spell
				if (se[i].caster == player)
					continue;

				luaspell = se[i].spell;
				spell = luaspell->spell;
				/* is this a friendly group spell? */
				if (spell && spell->GetSpellData()->group_spell && spell->GetSpellData()->friendly_spell) {

					player->GetSpellEffectMutex()->releasereadlock(__FUNCTION__, __LINE__);
					recoup_lock = true;
					// we have to remove our spell effect mutex lock since RemoveSpellEffect needs a write lock to remove it
					//Remove all group buffs not cast by this player
					player->RemoveSpellEffect(luaspell);
					player->RemoveSpellBonus(luaspell);
					player->RemoveSkillBonus(spell->GetSpellID());

					//Also remove group buffs from pets
					pet = 0;
					charmed_pet = 0;
					if (player->HasPet()) {
						pet = player->GetPet();
						pet = player->GetCharmedPet();
					}
					if (pet) {
						pet->RemoveSpellEffect(luaspell);
						pet->RemoveSpellBonus(luaspell);
					}
					if (charmed_pet) {
						charmed_pet->RemoveSpellEffect(luaspell);
						charmed_pet->RemoveSpellBonus(luaspell);
					}
				}
			}
		}
		if (!recoup_lock) { // we previously set a readlock that we now release
			player->GetSpellEffectMutex()->releasereadlock(__FUNCTION__, __LINE__);
		}
		packet = client->GetPlayer()->GetSkills()->GetSkillPacket(client->GetVersion());
		if (packet)
			client->QueuePacket(packet);
	}
	MGroups.unlock_shared();
}

int32 PlayerGroupManager::GetGroupSize(int32 group_id) {
	std::shared_lock lock(MGroups);
	int32 ret = 0;

	if (m_groups.count(group_id) > 0)
		ret = m_groups[group_id]->Size();

	return ret;
}

bool PlayerGroupManager::HasGroupCompletedQuest(int32 group_id, int32 quest_id) {
	std::shared_lock lock(MGroups);
	bool questComplete = true;
	GroupMemberInfo* info = 0;
	if (m_groups.count(group_id) > 0) {
		m_groups[group_id]->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = m_groups[group_id]->GetMembers();
		deque<GroupMemberInfo*>::iterator itr;
		for (itr = members->begin(); itr != members->end(); itr++) {
			info = *itr;
			if (info->client) {
				bool isComplete = info->client->GetPlayer()->HasQuestBeenCompleted(quest_id);
				if (!isComplete)
				{
					questComplete = isComplete;
					break;
				}
			}
		}
		m_groups[group_id]->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}
	return questComplete;
}

void PlayerGroupManager::SimpleGroupMessage(int32 group_id, const char* message) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(group_id) > 0)
		m_groups[group_id]->SimpleGroupMessage(message);
}

void PlayerGroupManager::SendGroupMessage(int32 group_id, int8 type, const char* message, ...) {
	std::shared_lock lock(MGroups);

	va_list argptr;
	char buffer[4096];
	buffer[0] = 0;
	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	if (m_groups.count(group_id) > 0)
		m_groups[group_id]->SendGroupMessage(type, buffer);
}

void PlayerGroupManager::GroupMessage(int32 group_id, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	buffer[0] = 0;
	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);
	SimpleGroupMessage(group_id, buffer);
}

void PlayerGroupManager::GroupChatMessage(int32 group_id, Spawn* from, int32 language, const char* message, int16 channel) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(group_id) > 0) {
		std::vector<int32> raidGroups;

		if (channel == CHANNEL_RAID_SAY)
			m_groups[group_id]->GetRaidGroups(&raidGroups);

		if (raidGroups.size() < 1)
			raidGroups.push_back(group_id);

		std::vector<int32>::iterator group_itr;
		for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
			if (m_groups.count((*group_itr))) {
				m_groups[(*group_itr)]->GroupChatMessage(from, language, message, channel);
			}
		}
	}
}

void PlayerGroupManager::GroupChatMessage(int32 group_id, std::string fromName, int32 language, const char* message, int16 channel) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(group_id) > 0) {
		std::vector<int32> raidGroups;

		if (channel == CHANNEL_RAID_SAY)
			m_groups[group_id]->GetRaidGroups(&raidGroups);

		if (raidGroups.size() < 1)
			raidGroups.push_back(group_id);

		std::vector<int32>::iterator group_itr;
		for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
			if (m_groups.count((*group_itr))) {
				m_groups[(*group_itr)]->GroupChatMessage(fromName, language, message, channel);
			}
		}
	}
}

void PlayerGroupManager::SendGroupChatMessage(int32 group_id, int16 channel, const char* message, ...) {
	std::shared_lock lock(MGroups);

	va_list argptr;
	char buffer[4096];
	buffer[0] = 0;
	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	if (m_groups.count(group_id) > 0) {
		std::vector<int32> raidGroups;

		if (channel == CHANNEL_RAID_SAY || channel == CHANNEL_LOOT_ROLLS || channel == CHANNEL_LOOT)
			m_groups[group_id]->GetRaidGroups(&raidGroups);

		if (raidGroups.size() < 1)
			raidGroups.push_back(group_id);

		std::vector<int32>::iterator group_itr;
		for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
			if (m_groups.count((*group_itr))) {
				m_groups[(*group_itr)]->SendGroupMessage(channel, buffer);
			}
		}
	}
}

bool PlayerGroupManager::MakeLeader(int32 group_id, Entity* new_leader) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(group_id) > 0)
		return m_groups[group_id]->MakeLeader(new_leader);

	return false;
}

void PlayerGroupManager::UpdateGroupBuffs() {
	map<int32, PlayerGroup*>::iterator itr;
	deque<GroupMemberInfo*>::iterator member_itr;
	deque<GroupMemberInfo*>::iterator target_itr;
	map<int32, SkillBonusValue*>::iterator itr_skills;
	MaintainedEffects* me = nullptr;
	LuaSpell* luaspell = nullptr;
	Spell* spell = nullptr;
	Entity* group_member = nullptr;
	SkillBonus* sb = nullptr;
	EQ2Packet* packet = nullptr;
	int32 i = 0;
	PlayerGroup* group = nullptr;
	Player* caster = nullptr;
	vector<int32> new_target_list;
	vector<int32> char_list;
	Client* client = nullptr;
	bool has_effect = false;
	vector<BonusValues*>* sb_list = nullptr;
	BonusValues* bv = nullptr;
	Entity* pet = nullptr;
	Entity* charmed_pet = nullptr;



	for (itr = m_groups.begin(); itr != m_groups.end(); itr++) {
		group = itr->second;

		/* loop through the group members and see if any of them have any maintained spells that are group buffs and friendly.
		if so, update the list of targets and apply/remove effects as needed */
		vector<Player*> players;

		group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		for (member_itr = group->GetMembers()->begin(); member_itr != group->GetMembers()->end(); member_itr++) {
			if ((*member_itr)->client)
				caster = (*member_itr)->client->GetPlayer();
			else caster = 0;

			if (!caster)
				continue;

			if (!caster->GetMaintainedSpellBySlot(0))
				continue;

			players.push_back(caster);
		}
		group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);

		vector<Player*>::iterator vitr;

		for (vitr = players.begin(); vitr != players.end(); vitr++) {
			caster = *vitr;
			caster->GetMaintainedMutex()->readlock(__FUNCTION__, __LINE__);
			// go through the player's maintained spells
			bool skipSpell = false;
			me = caster->GetMaintainedSpells();
			for (i = 0; i < NUM_MAINTAINED_EFFECTS; i++) {
				if (me[i].spell_id == 0xFFFFFFFF)
					continue;
				luaspell = me[i].spell;

				if (!luaspell)
					continue;

				if (!luaspell->caster)
				{
					LogWrite(PLAYER__ERROR, 0, "Player", "Bad luaspell, caster is NULL, spellid: %u", me[i].spell_id);
					continue;
				}

				spell = luaspell->spell;

				if (spell && spell->GetSpellData()->group_spell && spell->GetSpellData()->friendly_spell &&
					(spell->GetSpellData()->target_type == SPELL_TARGET_GROUP_AE || spell->GetSpellData()->target_type == SPELL_TARGET_RAID_AE)) {
					luaspell->ClearCharTargets();
					for (target_itr = group->GetMembers()->begin(); target_itr != group->GetMembers()->end(); target_itr++) {
						group_member = (*target_itr)->member;

						if (!group_member)
							continue;

						if (group_member == caster)
							continue;

						client = (*target_itr)->client;

						LuaSpell* conflictSpell = caster->HasLinkedTimerID(luaspell, group_member, false, true);
						if(conflictSpell && group_member && group_member->IsEntity())
						{
							if(conflictSpell->spell->GetSpellData()->min_class_skill_req > 0 && spell->GetSpellData()->min_class_skill_req > 0)
							{
								if(conflictSpell->spell->GetSpellData()->min_class_skill_req <= spell->GetSpellData()->min_class_skill_req)
								{
									if(spell->GetSpellData()->duration_until_cancel && !luaspell->num_triggers)
									{
										for (int32 id : conflictSpell->GetTargets()) {
											Spawn* tmpTarget = caster->GetZone()->GetSpawnByID(id);
											if(tmpTarget && tmpTarget->IsEntity())
											{
												((Entity*)tmpTarget)->RemoveEffectsFromLuaSpell(conflictSpell);
												caster->GetZone()->RemoveTargetFromSpell(conflictSpell, tmpTarget, false);
												caster->GetZone()->GetSpellProcess()->CheckRemoveTargetFromSpell(conflictSpell);
												lua_interface->RemoveSpawnFromSpell(conflictSpell, tmpTarget);
											}
										}
									}
								}
								else
								{
									// this is a spell that is no good, have to abort!
									caster->GetMaintainedMutex()->releasereadlock(__FUNCTION__, __LINE__);
									caster->GetZone()->GetSpellProcess()->SpellCannotStack(caster->GetZone(), client, caster, luaspell, conflictSpell);
									skipSpell = true;
									break;
								}
							}
						}
						has_effect = false;
						
						if (group_member->GetSpellEffect(spell->GetSpellID(), caster)) {
							has_effect = true;
						}
						std::vector<int32> removed_targets = luaspell->GetRemovedTargets();
						if (!has_effect && (std::find(removed_targets.begin(),
							removed_targets.end(), group_member->GetID()) != removed_targets.end())) {
							continue;
						}
						// Check if player is within range of the caster
						if (!rule_manager.GetGlobalRule(R_Spells, EnableCrossZoneGroupBuffs)->GetInt8() &&
							(group_member->GetZone() != caster->GetZone() || caster->GetDistance(group_member) > spell->GetSpellData()->radius)) {
							if (has_effect) {
								group_member->RemoveSpellEffect(luaspell);
								group_member->RemoveSpellBonus(luaspell);
								group_member->RemoveSkillBonus(spell->GetSpellID());
								if (client) {
									packet = ((Player*)group_member)->GetSkills()->GetSkillPacket(client->GetVersion());
									if (packet)
										client->QueuePacket(packet);
								}
								//Also remove group buffs from pet
								if (group_member->HasPet()) {
									pet = group_member->GetPet();
									charmed_pet = group_member->GetCharmedPet();
									if (pet) {
										pet->RemoveSpellEffect(luaspell);
										pet->RemoveSpellBonus(luaspell);
									}
									if (charmed_pet) {
										charmed_pet->RemoveSpellEffect(luaspell);
										charmed_pet->RemoveSpellBonus(luaspell);
									}
								}
							}
							continue;
						}

						if (group_member->GetZone() != caster->GetZone())
						{
							SpellProcess::AddSelfAndPetToCharTargets(luaspell, group_member);
						}
						else
						{
							//this group member is a target of the spell
							new_target_list.push_back(group_member->GetID());
						}

						if (has_effect)
							continue;

						pet = 0;
						charmed_pet = 0;

						if (group_member->HasPet()) {
							pet = group_member->GetPet();
							charmed_pet = group_member->GetCharmedPet();
						}

						group_member->AddSpellEffect(luaspell, luaspell->timer.GetRemainingTime() != 0 ? luaspell->timer.GetRemainingTime() : 0);
						if (pet)
							pet->AddSpellEffect(luaspell, luaspell->timer.GetRemainingTime() != 0 ? luaspell->timer.GetRemainingTime() : 0);
						if (charmed_pet)
							charmed_pet->AddSpellEffect(luaspell, luaspell->timer.GetRemainingTime() != 0 ? luaspell->timer.GetRemainingTime() : 0);

						if (pet)
							new_target_list.push_back(pet->GetID());
						if (charmed_pet)
							new_target_list.push_back(charmed_pet->GetID());


						// look for a spell bonus on caster's spell
						sb_list = caster->GetAllSpellBonuses(luaspell);
						for (int32 x = 0; x < sb_list->size(); x++) {
							bv = sb_list->at(x);
							group_member->AddSpellBonus(luaspell, bv->type, bv->value, bv->class_req, bv->race_req, bv->faction_req);
							if (pet)
								pet->AddSpellBonus(luaspell, bv->type, bv->value, bv->class_req, bv->race_req, bv->faction_req);
							if (charmed_pet)
								charmed_pet->AddSpellBonus(luaspell, bv->type, bv->value, bv->class_req, bv->race_req, bv->faction_req);
						}

						sb_list->clear();
						safe_delete(sb_list);

						// look for a skill bonus on the caster's spell
						sb = caster->GetSkillBonus(me[i].spell_id);
						if (sb) {
							for (itr_skills = sb->skills.begin(); itr_skills != sb->skills.end(); itr_skills++)
								group_member->AddSkillBonus(sb->spell_id, (*itr_skills).second->skill_id, (*itr_skills).second->value);
						}

						if (client) {
							packet = ((Player*)group_member)->GetSkills()->GetSkillPacket(client->GetVersion());
							if (packet)
								client->QueuePacket(packet);
						}
					}
					if(!skipSpell) {
						luaspell->SwapTargets(new_target_list);
						SpellProcess::AddSelfAndPet(luaspell, caster);
						new_target_list.clear();
					}
					else
						break;
				}
			}
			
			if(!skipSpell)
				caster->GetMaintainedMutex()->releasereadlock(__FUNCTION__, __LINE__);
		}
	}
}

bool PlayerGroupManager::IsInGroup(int32 group_id, Entity* member) {
	std::shared_lock lock(MGroups);
	bool ret = false;

	if (m_groups.count(group_id) > 0) {
		m_groups[group_id]->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = m_groups[group_id]->GetMembers();
		for (int8 i = 0; i < members->size(); i++) {
			if (member == members->at(i)->member) {
				ret = true;
				break;
			}
		}
		m_groups[group_id]->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}

	return ret;
}

Entity* PlayerGroupManager::IsPlayerInGroup(int32 group_id, int32 character_id) {
	std::shared_lock lock(MGroups);

	Entity* ret = nullptr;

	if (m_groups.count(group_id) > 0) {
		m_groups[group_id]->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = m_groups[group_id]->GetMembers();
		for (int8 i = 0; i < members->size(); i++) {
			if (members->at(i)->member && members->at(i)->member->IsPlayer() && character_id == ((Player*)members->at(i)->member)->GetCharacterID()) {
				ret = members->at(i)->member;
				break;
			}
		}
		m_groups[group_id]->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}

	return ret;
}

void PlayerGroupManager::SendPeerGroupData(std::string peerId) {
	if (peerId.size() < 1) // must provide a peerId
		return;

	std::shared_lock lock(MGroups); // unique lock to avoid updates while we push to a new peer
	map<int32, PlayerGroup*>::iterator itr;
	for (itr = m_groups.begin(); itr != m_groups.end(); itr++) {
		PlayerGroup* group = itr->second;
		if (group) {
			int32 groupID = group->GetID();
			std::vector<int32> raidGroups;
			group->GetRaidGroups(&raidGroups);
			peer_manager.sendPeersNewGroupRequest("", 0, groupID, "", "", group->GetGroupOptions(), peerId, &raidGroups);
			m_groups[groupID]->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = m_groups[groupID]->GetMembers();
			for (int8 i = 0; i < members->size(); i++) {
				GroupMemberInfo* info = members->at(i);
				if (info)
					peer_manager.sendPeersGroupMember(groupID, info, false, peerId);
			}
			m_groups[groupID]->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
		}
	}
}


void PlayerGroup::RemoveClientReference(Client* remove) {
	deque<GroupMemberInfo*>::iterator itr;
	MGroupMembers.writelock();
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		GroupMemberInfo* gmi = *itr;
		if (gmi->client && gmi->client == remove)
		{
			gmi->client = 0;
			gmi->member = 0;
			break;
		}
	}
	MGroupMembers.releasewritelock();
}

void PlayerGroup::UpdateGroupMemberInfo(Entity* ent, bool groupMembersLocked) {
	Player* player = (Player*)ent;

	if (!player || !player->GetGroupMemberInfo())
		return;

	if (!groupMembersLocked)
		MGroupMembers.writelock();

	GroupMemberInfo* group_member_info = player->GetGroupMemberInfo();
	player->GetGroupMemberInfo()->class_id = player->GetAdventureClass();
	group_member_info->hp_max = player->GetTotalHP();
	group_member_info->hp_current = player->GetHP();
	group_member_info->level_max = player->GetLevel();
	group_member_info->level_current = player->GetLevel();
	group_member_info->name = string(player->GetName());
	group_member_info->power_current = player->GetPower();
	group_member_info->power_max = player->GetTotalPower();
	group_member_info->race_id = player->GetRace();
	if (player->GetZone()) {
		group_member_info->zone = player->GetZone()->GetZoneDescription();
		group_member_info->zone_id = player->GetZone()->GetZoneID();
		group_member_info->instance_id = player->GetZone()->GetInstanceID();
	}
	else {
		group_member_info->zone = "Unknown";
		group_member_info->zone_id = 0;
		group_member_info->instance_id = 0;
	}

	peer_manager.sendPeersGroupMember(group_member_info->group_id, group_member_info, true);

	if (!groupMembersLocked)
		MGroupMembers.releasewritelock();
}

void PlayerGroup::GetRaidGroups(std::vector<int32>* groups) {
	std::shared_lock lock(MRaidGroups);
	if (groups)
		groups->insert(groups->end(), m_raidgroups.begin(), m_raidgroups.end());
}

void PlayerGroup::ReplaceRaidGroups(std::vector<int32>* groups) {
	std::unique_lock lock(MRaidGroups);
	if (groups) {
		m_raidgroups.clear();
		m_raidgroups.insert(m_raidgroups.end(), groups->begin(), groups->end());
	}
}

bool PlayerGroup::IsInRaidGroup(int32 groupID, bool isLeaderGroup) {
	std::unique_lock lock(MRaidGroups);
	if (isLeaderGroup) {
		if (m_raidgroups.size() > 0) {
			int32 group = m_raidgroups[0];
			if (group == groupID)
				return true;
		}
	}
	else if (std::find(m_raidgroups.begin(), m_raidgroups.end(), groupID) != m_raidgroups.end()) {
		return true;
	}
	return false;
}
void PlayerGroup::AddGroupToRaid(int32 groupID) {
	std::unique_lock lock(MRaidGroups);
	if (std::find(m_raidgroups.begin(), m_raidgroups.end(), groupID) == m_raidgroups.end()) {
		m_raidgroups.push_back(groupID);
	}
}

void PlayerGroup::RemoveGroupFromRaid(int32 groupID) {
	std::unique_lock lock(MRaidGroups);
	std::vector<int32>::iterator group_itr = std::find(m_raidgroups.begin(), m_raidgroups.end(), groupID);

	if (group_itr != m_raidgroups.end()) {
		m_raidgroups.erase(group_itr);
	}
}

bool PlayerGroup::IsGroupRaid() {
	std::unique_lock lock(MRaidGroups);
	return (m_raidgroups.size() > 1);
}

void PlayerGroup::ClearGroupRaid() {
	std::unique_lock lock(MRaidGroups);
	m_raidgroups.clear();
}

void PlayerGroupManager::UpdateGroupMemberInfoFromPeer(int32 group_id, std::string name, bool is_client, GroupMemberInfo* updateinfo) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(group_id) > 0) {
		m_groups[group_id]->MGroupMembers.writelock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = m_groups[group_id]->GetMembers();
		for (int8 i = 0; i < members->size(); i++) {
			GroupMemberInfo* curinfo = members->at(i);
			if (curinfo && curinfo->name == name && curinfo->is_client == is_client) {
				curinfo->class_id = updateinfo->class_id;
				curinfo->hp_max = updateinfo->hp_max;
				curinfo->hp_current = updateinfo->hp_current;
				curinfo->level_max = updateinfo->level_max;
				curinfo->level_current = updateinfo->level_current;
				curinfo->name = updateinfo->name;
				curinfo->power_current = updateinfo->power_current;
				curinfo->power_max = updateinfo->power_max;
				curinfo->race_id = updateinfo->race_id;
				curinfo->zone = updateinfo->zone;
				curinfo->mentor_target_char_id = updateinfo->mentor_target_char_id;
				curinfo->is_client = updateinfo->is_client;
				curinfo->leader = updateinfo->leader;
				curinfo->client_peer_address = updateinfo->client_peer_address;
				curinfo->client_peer_port = updateinfo->client_peer_port;
				curinfo->is_raid_looter = updateinfo->is_raid_looter;
				break;
			}
		}
		m_groups[group_id]->MGroupMembers.releasewritelock(__FUNCTION__, __LINE__);
	}
}

void PlayerGroupManager::ClearGroupRaid(int32 groupID) {
	std::shared_lock lock(MGroups);

	// Check to see if the id is in the list
	if (m_groups.count(groupID) > 0) {
		m_groups[groupID]->ClearGroupRaid();
	}
	SendGroupUpdate(groupID);
}

void PlayerGroupManager::RemoveGroupFromRaid(int32 groupID, int32 targetGroupID) {
	std::shared_lock lock(MGroups);

	// Check to see if the id is in the list
	if (m_groups.count(groupID) > 0) {
		// Get a pointer to the group
		PlayerGroup* group = m_groups[groupID];

		std::vector<int32> raidGroups;
		m_groups[groupID]->GetRaidGroups(&raidGroups);
		if (raidGroups.size() > 0) {
			std::vector<int32>::iterator group_itr;
			for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
				if (m_groups.count((*group_itr))) {
					m_groups[(*group_itr)]->RemoveGroupFromRaid(targetGroupID);
				}
			}
		}
	}
}

bool PlayerGroupManager::IsInRaidGroup(int32 groupID, int32 targetGroupID, bool isLeaderGroup) {
	std::shared_lock lock(MGroups);
	if (m_groups.count(groupID))
		return m_groups[groupID]->IsInRaidGroup(targetGroupID, isLeaderGroup);

	return false;
}

bool PlayerGroupManager::GetDefaultGroupOptions(int32 groupID, GroupOptions* options) {
	std::shared_lock lock(MGroups);
	if (m_groups.count(groupID))
		return m_groups[groupID]->GetDefaultGroupOptions(options);

	return false;
}

void PlayerGroupManager::GetRaidGroups(int32 groupID, std::vector<int32>* newGroups) {
	std::shared_lock lock(MGroups);
	if (m_groups.count(groupID))
		m_groups[groupID]->GetRaidGroups(newGroups);
}

void PlayerGroupManager::ReplaceRaidGroups(int32 groupID, std::vector<int32>* newGroups) {
	std::shared_lock lock(MGroups);
	if (m_groups.count(groupID))
		m_groups[groupID]->ReplaceRaidGroups(newGroups);

	SendGroupUpdate(groupID);
}

void PlayerGroupManager::SetGroupOptions(int32 groupID, GroupOptions* options) {
	std::shared_lock lock(MGroups);
	if (m_groups.count(groupID))
		m_groups[groupID]->SetDefaultGroupOptions(options);
}

void PlayerGroupManager::SendWhoGroupMembers(Client* client, int32 groupID) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(groupID) > 0) {
		PacketStruct* packet = configReader.getStruct("WS_WhoQueryReply", client->GetVersion());
		if (packet) {
			packet->setDataByName("account_id", client->GetAccountID());
			packet->setDataByName("unknown", 0xFFFFFFFF);
			int8 num_characters = 0;

			m_groups[groupID]->MGroupMembers.writelock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = m_groups[groupID]->GetMembers();
			for (int8 i = 0; i < members->size(); i++) {
				GroupMemberInfo* curinfo = members->at(i);
				if (curinfo && curinfo->name.size() > 0) {
					num_characters++;
				}
			}
			packet->setDataByName("response", 2);
			packet->setArrayLengthByName("num_characters", (int8)num_characters);
			for (int8 i = 0; i < members->size(); i++) {
				GroupMemberInfo* curinfo = members->at(i);
				if (curinfo && curinfo->name.size() > 0) {
					packet->setArrayDataByName("char_name", curinfo->name.c_str(), i);
					packet->setArrayDataByName("level", curinfo->level_current, i);
					packet->setArrayDataByName("class", curinfo->class_id, i);
					packet->setArrayDataByName("unknown4", 0xFF, i); //probably tradeskill class
					packet->setArrayDataByName("race", curinfo->race_id, i);
					packet->setArrayDataByName("zone", curinfo->zone.c_str(), i);
				}
			}
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
		m_groups[groupID]->MGroupMembers.releasewritelock(__FUNCTION__, __LINE__);
	}
}

void PlayerGroupManager::SendWhoRaidMembers(Client* client, int32 groupID) {
	std::shared_lock lock(MGroups);

	PlayerGroup* group = nullptr;
	if (m_groups.count(groupID))
		group = m_groups[groupID];

	std::vector<int32> groups;
	std::vector<int32>::iterator group_itr;
	if (group)
		group->GetRaidGroups(&groups);

	if (groups.size() > 0) {
		PacketStruct* packet = configReader.getStruct("WS_WhoQueryReply", client->GetVersion());
		if (packet) {
			packet->setDataByName("account_id", client->GetAccountID());
			packet->setDataByName("unknown", 0xFFFFFFFF);
			int8 num_characters = 0;
			for (group_itr = groups.begin(); group_itr != groups.end(); group_itr++) {
				if (m_groups.count((*group_itr))) {
					// we will release this lock after we submit the data out below in the second group_itr
					m_groups[(*group_itr)]->MGroupMembers.readlock(__FUNCTION__, __LINE__);
					deque<GroupMemberInfo*>* members = m_groups[(*group_itr)]->GetMembers();
					for (int8 i = 0; i < members->size(); i++) {
						GroupMemberInfo* curinfo = members->at(i);
						if (curinfo && curinfo->name.size() > 0) {
							num_characters++;
						}
					}
				}
			}
			packet->setDataByName("response", 2);
			packet->setArrayLengthByName("num_characters", (int8)num_characters);
			int8 pos = 0;
			for (group_itr = groups.begin(); group_itr != groups.end(); group_itr++) {
				if (m_groups.count((*group_itr))) {
					deque<GroupMemberInfo*>* members = m_groups[(*group_itr)]->GetMembers();
					for (int8 i = 0; i < members->size(); i++) {
						GroupMemberInfo* curinfo = members->at(i);
						if (curinfo && curinfo->name.size() > 0) {
							packet->setArrayDataByName("char_name", curinfo->name.c_str(), pos);
							packet->setArrayDataByName("level", curinfo->level_current, pos);
							packet->setArrayDataByName("class", curinfo->class_id, pos);
							packet->setArrayDataByName("unknown4", 0xFF, pos); //probably tradeskill class
							packet->setArrayDataByName("race", curinfo->race_id, pos);
							packet->setArrayDataByName("zone", curinfo->zone.c_str(), pos);
							pos++;
						}
					}
					// release previously established lock during the count
					m_groups[(*group_itr)]->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
				}
			}
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	else {
		client->SimpleMessage(CHANNEL_COLOR_RED, "You are not currently in a raid.");
	}
}

int8 PlayerGroupManager::AcceptRaidInvite(std::string acceptorName, int32 groupID) {
	std::unique_lock lock(MGroups);

	std::string raidLeaderName("");
	MPendingInvites.writelock(__FUNCTION__, __LINE__);
	bool inviteAccept = true;
	if (m_raidPendingInvites.count(acceptorName.c_str()) > 0) {
		raidLeaderName = m_raidPendingInvites[acceptorName.c_str()];
		m_raidPendingInvites.erase(acceptorName.c_str());
	}
	MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);
	if (raidLeaderName.size() < 1)
		return 0;

	Client* client_leader = zone_list.GetClientByCharName(raidLeaderName.c_str());
	if (client_leader) {
		if (client_leader->GetPlayer()->GetGroupMemberInfo() && m_groups.count(groupID) && m_groups.count(client_leader->GetPlayer()->GetGroupMemberInfo()->group_id)) {
			Player* player = client_leader->GetPlayer();
			PlayerGroup* spawn_group = m_groups[groupID];
			PlayerGroup* player_group = m_groups[player->GetGroupMemberInfo()->group_id];
			if (spawn_group && player_group) {
				std::vector<int32> groups;
				spawn_group->GetRaidGroups(&groups);
				if (groups.size() > 0) {
					return 0;
				}

				player_group->GetRaidGroups(&groups);
				if (groups.size() > 3) {
					return 0;
				}

				spawn_group->SetDefaultGroupOptions(player_group->GetGroupOptions());
				if (groups.size() < 1) {
					player_group->AddGroupToRaid(player_group->GetID());
					player_group->AddGroupToRaid(spawn_group->GetID());
					player_group->GetRaidGroups(&groups);
					spawn_group->ReplaceRaidGroups(&groups);
					return 1;
				}
				else {
					groups.clear();
					player_group->AddGroupToRaid(spawn_group->GetID());
					player_group->GetRaidGroups(&groups);
					std::vector<int32>::iterator group_itr;
					for (group_itr = groups.begin(); group_itr != groups.end(); group_itr++) {
						int32 cur_group_id = (*group_itr);
						if (cur_group_id == player_group->GetID() || m_groups.count(cur_group_id) < 1) // we already set the player_group above, skip
							continue;
						PlayerGroup* temp_group = m_groups[cur_group_id];
						temp_group->ReplaceRaidGroups(&groups);
					}
					return 1;
				}

			}
		}
	}
	else {
		// must be somewhere else in the world
	}
	return 0;
}

bool PlayerGroupManager::SendRaidInvite(Client* sender, Entity* target) {
	std::shared_lock lock(MGroups);
	if (!sender || !target)
		return false;

	if (!target->IsPlayer() || !((Player*)target)->GetClient() || !target->GetGroupMemberInfo() || !sender->GetPlayer()->GetGroupMemberInfo())
		return false;

	Player* player = sender->GetPlayer();
	int32 spawn_group_id = target->GetGroupMemberInfo()->group_id;

	if (m_groups.count(spawn_group_id) < 1)
		return false;

	PlayerGroup* spawn_group = m_groups[spawn_group_id];

	int32 player_group_id = player->GetGroupMemberInfo() ? player->GetGroupMemberInfo()->group_id : 0;
	PlayerGroup* player_group = nullptr;
	if (m_groups.count(player_group_id))
		player_group = m_groups[player_group_id];
	// check if already invited
	if (spawn_group && !player->IsGroupMember((Player*)target) && !spawn_group->IsGroupRaid() && player_group && player->GetGroupMemberInfo()->leader
		&& (!player_group->IsInRaidGroup(spawn_group_id) || player_group->IsInRaidGroup(player_group_id, true))) {
		std::string leader = spawn_group->GetLeaderName();
		std::vector<int32> groups;
		player_group->GetRaidGroups(&groups);
		if (groups.size() > 3) {
			sender->SimpleMessage(CHANNEL_COLOR_RED, "You are currently in a full raid.");
			return false;
		}
		if (leader.size() < 1) {
			sender->SimpleMessage(CHANNEL_COLOR_RED, "Your target has no leader.");
			return false;
		}
		MPendingInvites.readlock(__FUNCTION__, __LINE__);
		bool inviteAccept = true;
		if (m_raidPendingInvites.count(leader.c_str()) > 0) {
			inviteAccept = false;
			sender->SimpleMessage(CHANNEL_COLOR_RED, "Leader of the other group has a pending raid invite.");
		}
		MPendingInvites.releasereadlock(__FUNCTION__, __LINE__);

		if (!inviteAccept)
			return false;

		MPendingInvites.writelock(__FUNCTION__, __LINE__);
		m_raidPendingInvites[leader] = player->GetGroupMemberInfo()->name.c_str();
		MPendingInvites.releasewritelock(__FUNCTION__, __LINE__);

		sender->SendReceiveOffer(((Player*)target)->GetClient(), 3, std::string(sender->GetPlayer()->GetName()), 1);
		return true;
	}
	return false;
}

void PlayerGroupManager::SplitWithGroupOrRaid(Client* client, int32 coin_plat, int32 coin_gold, int32 coin_silver, int32 coin_copper) {
	std::shared_lock lock(MGroups);
	bool startWithLooter = true;
	if (!client->GetPlayer()->GetGroupMemberInfo())
		return;
	if (!coin_plat && !coin_gold && !coin_silver && !coin_copper)
		return;
	if (client->GetPlayer()->GetInfoStruct()->get_coin_plat() < coin_plat &&
		client->GetPlayer()->GetInfoStruct()->get_coin_gold() < coin_gold &&
		client->GetPlayer()->GetInfoStruct()->get_coin_silver() < coin_silver &&
		client->GetPlayer()->GetInfoStruct()->get_coin_copper() < coin_copper) {
		// lacking coin
		return;
	}
	int32 groupID = client->GetPlayer()->GetGroupMemberInfo()->group_id;

	if (m_groups.count(groupID) < 1) {
		return;
	}

	PlayerGroup* group = m_groups[groupID];
	if (group)
	{
		bool isLeadGroup = group->IsInRaidGroup(group->GetID(), true);
		bool isInRaid = group->IsInRaidGroup(group->GetID());
		std::vector<int32> raidGroups;
		group->GetRaidGroups(&raidGroups);

		if (!isInRaid && raidGroups.size() < 1) {
			raidGroups.push_back(group->GetID());
		}
		std::vector<int32>::iterator group_itr;

		int32 split_coin_per_player = 0;
		int32 actual_coins = coin_plat * 1000000 + coin_gold * 10000 + coin_silver * 100 + coin_copper;
		int32 coins_remain_after_split = actual_coins;
		int32 total_coins = actual_coins;

		bool foundLooterResetRaidRun = false;
		int8 members_in_zone = 0;
		for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
			group = m_groups[(*group_itr)];
			if (!group)
				continue;

			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();
			for (int8 i = 0; i < members->size(); i++) {
				Entity* member = members->at(i)->member;
				if (!member || !member->IsPlayer())
					continue;
				if (member->GetZone() != client->GetPlayer()->GetZone())
					continue;

				members_in_zone++;
			}
			group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
		}
		if (members_in_zone < 1) // this should not happen, but divide by zero checked
			members_in_zone = 1;
		split_coin_per_player = actual_coins / members_in_zone;
		coins_remain_after_split = actual_coins - (split_coin_per_player * members_in_zone);
		// try to individually take each tier of coin and not split the coin in inventory
		client->GetPlayer()->GetInfoStruct()->set_coin_plat(client->GetPlayer()->GetInfoStruct()->get_coin_plat() - coin_plat);
		client->GetPlayer()->GetInfoStruct()->set_coin_gold(client->GetPlayer()->GetInfoStruct()->get_coin_gold() - coin_gold);
		client->GetPlayer()->GetInfoStruct()->set_coin_silver(client->GetPlayer()->GetInfoStruct()->get_coin_silver() - coin_silver);
		client->GetPlayer()->GetInfoStruct()->set_coin_copper(client->GetPlayer()->GetInfoStruct()->get_coin_copper() - coin_copper);
		int32 lootGroup = 0;
		for (group_itr = raidGroups.begin(); group_itr != raidGroups.end();) {
			group = m_groups[(*group_itr)];
			if (!group)
				continue;

			isLeadGroup = group->IsInRaidGroup((*group_itr), true);

			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();

			LogWrite(LOOT__INFO, 0, "Loot", "%s: Group SplitWithGroupOrRaid, split coin per player %u, remaining coin after split %u", client->GetPlayer()->GetName(), split_coin_per_player, coins_remain_after_split);
			for (int8 i = 0; i < members->size(); i++) {
				Entity* member = members->at(i)->member;
				if (!member || !member->IsPlayer())
					continue;

				if (member->GetZone() != client->GetPlayer()->GetZone())
					continue;

				// this will make sure we properly send the loot window to the initial requester if there is no item rarity matches
				if (startWithLooter && member != client->GetPlayer())
					continue;
				else if (!startWithLooter && member == client->GetPlayer())
					continue;

				int32 coin_recv = 0;
				if (member == client->GetPlayer() && (split_coin_per_player + coins_remain_after_split) > 0) {
					coin_recv = split_coin_per_player + coins_remain_after_split;
					((Player*)member)->AddCoins(split_coin_per_player + coins_remain_after_split);
					if (coins_remain_after_split > 0) // overflow of coin division went to the first player
						coins_remain_after_split = 0;
				}
				else if (split_coin_per_player > 0) {
					coin_recv = split_coin_per_player;
					((Player*)member)->AddCoins(split_coin_per_player);
				}
				if (coin_recv && ((Player*)member)->GetClient()) {
					((Player*)member)->GetClient()->Message(CHANNEL_MONEY_SPLIT, "Your share of the %s split from %s is %s.", client->GetCoinMessage(total_coins).c_str(), client->GetPlayer()->GetName(), client->GetCoinMessage(coin_recv).c_str());
				}
				if (startWithLooter) {
					startWithLooter = false;
					foundLooterResetRaidRun = true; // we got it, shouldn't hit this again
					break;
				}
			}
			group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
			if (foundLooterResetRaidRun) {
				group_itr = raidGroups.begin();
				foundLooterResetRaidRun = false; // disable running it again
			}
			else
				group_itr++;
		} // end raid groups
	} // end group 
}

bool PlayerGroupManager::IdentifyMemberInGroupOrRaid(ZoneChangeDetails* details, Client* client, int32 zoneID, int32 instanceID) {
	std::shared_lock lock(MGroups);
	ZoneServer* ret = nullptr;
	PlayerGroup* group = nullptr;
	bool succeed = false;
	if (client->GetPlayer()->GetGroupMemberInfo() && m_groups.count(client->GetPlayer()->GetGroupMemberInfo()->group_id))
		group = m_groups[client->GetPlayer()->GetGroupMemberInfo()->group_id];
	else
		return false;

	std::vector<int32> raidGroups;
	std::vector<int32>::iterator group_itr;
	if (group)
		group->GetRaidGroups(&raidGroups);

	group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
	deque<GroupMemberInfo*>* members = group->GetMembers();
	deque<GroupMemberInfo*>::iterator itr;
	for (itr = members->begin(); itr != members->end(); itr++) {
		// If a group member matches a target
		if ((*itr)->is_client && (*itr)->member && (*itr)->member->GetZone() && (*itr)->zone_id == zoneID && (instanceID == 0 || (*itr)->instance_id == instanceID)) {
			// toggle the flag and break the loop
			ret = (*itr)->member->GetZone();
			break;
		}
		else if ((*itr)->is_client && (*itr)->zone_id == zoneID  && (instanceID == 0 || (*itr)->instance_id == instanceID)) {
			// toggle the flag and break the loop		
			std::string id = peer_manager.isPeer((*itr)->client_peer_address, (*itr)->client_peer_port);
			std::shared_ptr<Peer> peer = peer_manager.getPeerById(id);
			if (peer) {
				ZoneServer* tmp = new ZoneServer((*itr)->zone.c_str());
				database.LoadZoneInfo(tmp);
				peer_manager.setZonePeerData(details, peer->id, peer->worldAddr, peer->internalWorldAddr, peer->worldPort, peer->webAddr, peer->webPort, std::string(tmp->GetZoneFile()), std::string(tmp->GetZoneName()), tmp->GetZoneID(),
					tmp->GetInstanceID(), tmp->GetSafeX(), tmp->GetSafeY(), tmp->GetSafeZ(), tmp->GetSafeHeading(),
					tmp->GetZoneLockState(), tmp->GetMinimumStatus(), tmp->GetMinimumLevel(), tmp->GetMaximumLevel(),
					tmp->GetMinimumVersion(), tmp->GetDefaultLockoutTime(), tmp->GetDefaultReenterTime(),
					tmp->GetInstanceType(), tmp->NumPlayers(), tmp->IsCityZone());
				safe_delete(tmp);
				succeed = true;
				break;
			}
		}
	}
	group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	if (succeed) {
		return true;
	}
	else if (ret) {
		peer_manager.setZonePeerDataSelf(details, std::string(ret->GetZoneFile()), std::string(ret->GetZoneName()),
			ret->GetZoneID(), ret->GetInstanceID(), ret->GetSafeX(), ret->GetSafeY(),
			ret->GetSafeZ(), ret->GetSafeHeading(), ret->GetZoneLockState(),
			ret->GetMinimumStatus(), ret->GetMinimumLevel(), ret->GetMaximumLevel(),
			ret->GetMinimumVersion(), ret->GetDefaultLockoutTime(), ret->GetDefaultReenterTime(),
			ret->GetInstanceType(), ret->NumPlayers(), ret->IsCityZone(), ret);
		return true;
	}


	for (group_itr = raidGroups.begin(); group_itr != raidGroups.end(); group_itr++) {
		if (m_groups.count((*group_itr))) {
			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();
			deque<GroupMemberInfo*>::iterator itr;
			for (itr = members->begin(); itr != members->end(); itr++) {
				// If a group member matches a target
				if ((*itr)->is_client && (*itr)->zone_id == zoneID && (*itr)->instance_id == instanceID) {
					// toggle the flag and break the loop

					std::string id = peer_manager.isPeer((*itr)->client_peer_address, (*itr)->client_peer_port);
					std::shared_ptr<Peer> peer = peer_manager.getPeerById(id);
					if (peer) {
						ZoneServer* tmp = new ZoneServer((*itr)->zone.c_str());
						database.LoadZoneInfo(tmp);
						peer_manager.setZonePeerData(details, peer->id, peer->worldAddr, peer->internalWorldAddr, peer->worldPort, peer->webAddr, peer->webPort, std::string(tmp->GetZoneFile()), std::string(tmp->GetZoneName()), tmp->GetZoneID(),
							tmp->GetInstanceID(), tmp->GetSafeX(), tmp->GetSafeY(), tmp->GetSafeZ(), tmp->GetSafeHeading(),
							tmp->GetZoneLockState(), tmp->GetMinimumStatus(), tmp->GetMinimumLevel(), tmp->GetMaximumLevel(),
							tmp->GetMinimumVersion(), tmp->GetDefaultLockoutTime(), tmp->GetDefaultReenterTime(),
							tmp->GetInstanceType(), tmp->NumPlayers(), tmp->IsCityZone());
						safe_delete(tmp);
						succeed = true;
						break;
					}
				}
			}
			group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
			if (succeed)
				break;
		}
	}
	return succeed;
}

void PlayerGroupManager::ClearGroupRaidLooterFlag(int32 groupID) {
	std::shared_lock lock(MGroups);

	if (m_groups.count(groupID) > 0) {
		m_groups[groupID]->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = m_groups[groupID]->GetMembers();
		for (int8 i = 0; i < members->size(); i++) {
			GroupMemberInfo* curinfo = members->at(i);
			if (curinfo) {
				curinfo->is_raid_looter = false;
			}
		}
		m_groups[groupID]->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}
}

void PlayerGroupManager::EstablishRaidLevelRange(Client* client, int32* min_level, int32* max_level, int32* avg_level, int32* first_level) {
	std::shared_lock lock(MGroups);
	if(!client)
		return;
	
	if (!client->GetPlayer()->GetGroupMemberInfo()) {
		*min_level = *max_level = *avg_level = *first_level = client->GetPlayer()->GetLevel();
		return;
	}

	int32 groupID = client->GetPlayer()->GetGroupMemberInfo()->group_id;

	if (m_groups.count(groupID) < 1) {
		*min_level = *max_level = *avg_level = *first_level = client->GetPlayer()->GetLevel();
		return;
	}

	PlayerGroup* group = m_groups[groupID];
	if (group) {
		bool isInRaid = group->IsInRaidGroup(group->GetID());
		std::vector<int32> raidGroups;
		group->GetRaidGroups(&raidGroups);

		if (!isInRaid && raidGroups.size() < 1) {
			raidGroups.push_back(group->GetID());
		}

		// Initialize tracking variables
		int32 local_min_level = INT32_MAX;
		int32 local_max_level = INT32_MIN;
		int32 total_levels = 0;
		int8 level_count = 0;

		// Get the first player's level
		*first_level = client->GetPlayer()->GetLevel();

		for (auto& groupID : raidGroups) {
			group = m_groups[groupID];
			if (!group)
				continue;

			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();

			for (auto& memberInfo : *members) {
				Entity* member = memberInfo->member;
				if (!member || !member->IsPlayer())
					continue;

				if (member->GetZone() != client->GetPlayer()->GetZone())
					continue;

				int32 member_level = member->GetLevel();
				local_min_level = std::min(local_min_level, member_level);
				local_max_level = std::max(local_max_level, member_level);
				total_levels += member_level;
				level_count++;
			}
			group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
		}

		// Finalize values with divide-by-zero protection
		if (level_count > 0) {
			*min_level = local_min_level;
			*max_level = local_max_level;
			*avg_level = total_levels / level_count;
		}
		else {
			*min_level = *max_level = *avg_level = *first_level = client->GetPlayer()->GetLevel();
		}
	}
	else {
		*min_level = *max_level = *avg_level = *first_level = client->GetPlayer()->GetLevel();
	}
}

Entity* PlayerGroup::GetGroupMemberByPosition(Entity* seeker, int32 mapped_position) {
	Entity* ret = nullptr;

	deque<GroupMemberInfo*>::iterator itr;

	MGroupMembers.readlock();

	int32 count = 1;
	for (itr = m_members.begin(); itr != m_members.end(); itr++) {
		if ((*itr)->member == seeker) {
			continue;
		}
		count++;
		if (count >= mapped_position) {
			ret = (Entity*)(*itr)->member;
			break;
		}
	}

	MGroupMembers.releasereadlock();

	return ret;
}

void PlayerGroup::SetDefaultGroupOptions(GroupOptions* options) {
	MGroupMembers.writelock();
	if (options != nullptr) {
		group_options.loot_method = options->loot_method;
		group_options.loot_items_rarity = options->loot_items_rarity;
		group_options.auto_split = options->auto_split;
		group_options.default_yell = options->default_yell;
		group_options.group_lock_method = options->group_lock_method;
		group_options.group_autolock = options->group_autolock;
		group_options.solo_autolock = options->solo_autolock;
		group_options.auto_loot_method = options->auto_loot_method;
	}
	else {
		group_options.loot_method = 1;
		group_options.loot_items_rarity = 0;
		group_options.auto_split = 1;
		group_options.default_yell = 1;
		group_options.group_lock_method = 0;
		group_options.group_autolock = 0;
		group_options.solo_autolock = 0;
		group_options.auto_loot_method = 0;
		group_options.last_looted_index = 0;
	}

	MGroupMembers.releasewritelock();
}

bool PlayerGroup::GetDefaultGroupOptions(GroupOptions* options) {
	bool setOptions = false;
	MGroupMembers.readlock();
	if (options != nullptr) {
		options->loot_method = group_options.loot_method;
		options->loot_items_rarity = group_options.loot_items_rarity;
		options->auto_split = group_options.auto_split;
		options->default_yell = group_options.default_yell;
		options->group_lock_method = group_options.group_lock_method;
		options->group_autolock = group_options.group_autolock;
		options->solo_autolock = group_options.solo_autolock;
		options->auto_loot_method = group_options.auto_loot_method;
		setOptions = true;
	}
	MGroupMembers.releasereadlock();
	return setOptions;
}
