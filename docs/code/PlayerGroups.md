# File: `PlayerGroups.h`

## Classes

- `GroupOptions`
- `GroupMemberInfo`
- `PlayerGroup`
- `PlayerGroupManager`

## Functions

- `bool AddMember(Entity* member, bool is_leader);`
- `bool RemoveMember(Entity* member);`
- `bool RemoveMember(std::string name, bool is_client, int32 charID);`
- `void Disband();`
- `void SendGroupUpdate(Client* exclude = 0, bool forceRaidUpdate = false);`
- `int32 Size() { return m_members.size(); }`
- `void SimpleGroupMessage(const char* message);`
- `void SendGroupMessage(int8 type, const char* message, ...);`
- `void GroupChatMessage(Spawn* from, int32 language, const char* message, int16 channel = CHANNEL_GROUP_SAY);`
- `void GroupChatMessage(std::string fromName, int32 language, const char* message, int16 channel = CHANNEL_GROUP_SAY);`
- `bool MakeLeader(Entity* new_leader);`
- `std::string GetLeaderName();`
- `bool ShareQuestWithGroup(Client* quest_sharer, Quest* quest);`
- `void RemoveClientReference(Client* remove);`
- `void UpdateGroupMemberInfo(Entity* ent, bool groupMembersLocked = false);`
- `void SetDefaultGroupOptions(GroupOptions* options = nullptr);`
- `bool GetDefaultGroupOptions(GroupOptions* options);`
- `int8 GetLastLooterIndex() { return group_options.last_looted_index; }`
- `void SetNextLooterIndex(int8 new_index) { group_options.last_looted_index = new_index; }`
- `int32 GetID() { return m_id; }`
- `void GetRaidGroups(std::vector<int32>* groups);`
- `void ReplaceRaidGroups(std::vector<int32>* groups);`
- `bool IsInRaidGroup(int32 groupID, bool isLeaderGroup = false);`
- `void AddGroupToRaid(int32 groupID);`
- `void RemoveGroupFromRaid(int32 groupID);`
- `bool IsGroupRaid();`
- `void ClearGroupRaid();`
- `bool AddGroupMember(int32 group_id, Entity* member, bool is_leader = false);`
- `bool AddGroupMemberFromPeer(int32 group_id, GroupMemberInfo* info);`
- `bool RemoveGroupMember(int32 group_id, Entity* member);`
- `bool RemoveGroupMember(int32 group_id, std::string name, bool is_client, int32 charID);`
- `int32 NewGroup(Entity* leader, GroupOptions* goptions, int32 override_group_id = 0);`
- `void RemoveGroup(int32 group_id);`
- `int8 Invite(Player* leader, Entity* member);`
- `bool AddInvite(Player* leader, Entity* member);`
- `int8 AcceptInvite(Entity* member, int32* group_override_id = nullptr, bool auto_add_group = true);`
- `void DeclineInvite(Entity* member);`
- `bool IsGroupIDValid(int32 group_id);`
- `void SendGroupUpdate(int32 group_id, Client* exclude = 0, bool forceRaidUpdate = false);`
- `void GroupHardLock(const char* function = 0, int32 line = 0U) { MGroups.lock(); }`
- `void GroupLock(const char* function = 0, int32 line = 0U) { MGroups.lock_shared(); }`
- `void ReleaseGroupHardLock(const char* function = 0, int32 line = 0U) { MGroups.unlock(); }`
- `void ReleaseGroupLock(const char* function = 0, int32 line = 0U) { MGroups.unlock_shared(); }`
- `void ClearPendingInvite(Entity* member);`
- `std::string HasPendingInvite(Entity* member);`
- `void RemoveGroupBuffs(int32 group_id, Client* client);`
- `int32 GetGroupSize(int32 group_id);`
- `void SendGroupQuests(int32 group_id, Client* client);`
- `bool HasGroupCompletedQuest(int32 group_id, int32 quest_id);`
- `void SimpleGroupMessage(int32 group_id, const char* message);`
- `void SendGroupMessage(int32 group_id, int8 type, const char* message, ...);`
- `void GroupMessage(int32 group_id, const char* message, ...);`
- `void GroupChatMessage(int32 group_id, Spawn* from, int32 language, const char* message, int16 channel = CHANNEL_GROUP_SAY);`
- `void GroupChatMessage(int32 group_id, std::string fromName, int32 language, const char* message, int16 channel = CHANNEL_GROUP_SAY);`
- `void SendGroupChatMessage(int32 group_id, int16 channel, const char* message, ...);`
- `bool MakeLeader(int32 group_id, Entity* new_leader);`
- `void UpdateGroupBuffs();`
- `bool IsInGroup(int32 group_id, Entity* member);`
- `bool IsSpawnInGroup(int32 group_id, string name); // used in follow`
- `void UpdateGroupMemberInfoFromPeer(int32 group_id, std::string name, bool is_client, GroupMemberInfo* updateinfo);`
- `void SendPeerGroupData(std::string peerId);`
- `void ClearGroupRaid(int32 groupID);`
- `void RemoveGroupFromRaid(int32 groupID, int32 targetGroupID);`
- `bool IsInRaidGroup(int32 groupID, int32 targetGroupID, bool isLeaderGroup = false);`
- `bool GetDefaultGroupOptions(int32 group_id, GroupOptions* options);`
- `void GetRaidGroups(int32 group_id, std::vector<int32>* groups);`
- `void ReplaceRaidGroups(int32 groupID, std::vector<int32>* newGroups);`
- `void SetGroupOptions(int32 groupID, GroupOptions* options);`
- `void SendWhoGroupMembers(Client* client, int32 groupID);`
- `void SendWhoRaidMembers(Client* client, int32 groupID);`
- `int8 AcceptRaidInvite(std::string acceptorName, int32 groupID);`
- `bool SendRaidInvite(Client* sender, Entity* target);`
- `void SplitWithGroupOrRaid(Client* client, int32 coin_plat, int32 coin_gold, int32 coin_silver, int32 coin_copper);`
- `bool IdentifyMemberInGroupOrRaid(ZoneChangeDetails* details, Client* client, int32 zoneID, int32 instanceID = 0);`
- `void ClearGroupRaidLooterFlag(int32 groupID);`
- `void EstablishRaidLevelRange(Client* client, int32* min_level, int32* max_level, int32* avg_level, int32* first_level);`

## Notable Comments

- /*
- */
- // GroupOptions isn't used yet
- /// <summary>All the generic info for the group window, plus a client pointer for players</summary>
- /// <summary>Represents a players group in game</summary>
- /// <summary>Adds a new member to the players group</summary>
- /// <param name='member'>Entity to add to the group, can be a Player or NPC</param>
- /// <returns>True if the member was added</returns>
- /// <summary>Removes a member from the players group</summary>
- /// <param name='member'>Entity to remove from the player group</param>
