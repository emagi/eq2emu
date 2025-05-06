# File: `PeerManager.h`

## Classes

- `Client`
- `HealthCheck`
- `GroupOptions`
- `GroupMemberInfo`
- `WhoAllPeerPlayer`
- `GuildMember`
- `ZoneChangeDetails`
- `Peer`
- `PeerManager`

## Functions

- `void updateStatus(HealthStatus newStatus);`
- `std::chrono::duration<double> timeSinceLastCheck() const;`
- `: zone_tree(std::make_shared<boost::property_tree::ptree>()),`
- `: id(std::move(peerId)), peeringStatus(status), worldAddr(std::move(client_address)),`
- `std::string getZoneDataAsJson() const {`
- `std::lock_guard<std::mutex> lock(dataMutex);`
- `std::string getClientDataAsJson() const {`
- `std::lock_guard<std::mutex> lock(dataMutex);`
- `void addPeer(std::string id, PeeringStatus status, std::string client_address, std::string client_internal_address, int16 client_port, std::string web_address, int16 web_port);`
- `void updateHealth(const std::string& id, HealthStatus newStatus);`
- `void updatePriority(const std::string& id, int16 priority);`
- `void updateZoneTree(const std::string& id, const boost::property_tree::ptree& newTree);`
- `void updateClientTree(const std::string& id, const boost::property_tree::ptree& newTree);`
- `bool IsClientConnectedPeer(int32 account_id);`
- `std::string GetCharacterPeerId(std::string charName);`
- `void SendPeersChannelMessage(int32 group_id, std::string fromName, std::string message, int16 channel, int32 language_id = 0);`
- `void SendPeersGuildChannelMessage(int32 guild_id, std::string fromName, std::string message, int16 channel, int32 language_id = 0);`
- `void sendZonePeerList(Client* client);`
- `std::string getZonePeerId(const std::string& inc_zone_name, int32 inc_zone_id, int32 inc_instance_id, ZoneChangeDetails* opt_details = nullptr, bool only_always_loaded = false, int32 matchDuplicatedId = 0);`
- `int32 getZoneHighestDuplicateId(const std::string& inc_zone_name, int32 inc_zone_id, bool increment_new_value = true);`
- `void setZonePeerData(ZoneChangeDetails* opt_details);`
- `void setPrimary(const std::string& id);`
- `bool hasPrimary();`
- `bool hasPriorityPeer(int16 priority);`
- `std::string getPriorityPeer();`
- `void updatePeer(const std::string& web_address, int16 web_port, const std::string& client_address, const std::string& client_internal_address, int16 client_port, bool is_primary = false);`
- `std::string isPeer(const std::string& web_address, int16 web_port);`
- `HealthStatus getPeerStatus(const std::string& web_address, int16 web_port);`
- `bool hasPeers();`
- `std::string assignUniqueNameForSecondary(const std::string& baseName, std::string client_address, std::string client_internal_address, int16 client_port, std::string web_address, int16 web_port);`
- `std::optional<std::string> getHealthyPeer() const;`
- `std::shared_ptr<Peer> getHealthyPeerPtr() const;`
- `std::shared_ptr<Peer> getHealthyPrimaryPeerPtr() const;`
- `std::shared_ptr<Peer> getHealthyPeerWithLeastClients() const;`
- `std::shared_ptr<Peer> getPeerById(const std::string& id) const;`
- `int32 getUniqueGroupId();`
- `bool sendPrimaryNewGroupRequest(std::string leader, std::string member, int32 entity_id, GroupOptions* options);`
- `void sendPeersGroupMember(int32 group_id, GroupMemberInfo* info, bool is_update = false, std::string peerId = "");`
- `void sendPeersRemoveGroupMember(int32 group_id, std::string name, int32 char_id, bool is_client);`
- `void populateGroupOptions(boost::property_tree::ptree& root, GroupOptions* options);`
- `void sendPeersDisbandGroup(int32 group_id);`
- `bool sendPrimaryCreateGuildRequest(std::string guild_name, std::string leader_name, bool prompted_dialog = false, int32 spawnID = 0);`
- `void sendPeersAddGuildMember(int32 character_id, int32 guild_id, std::string invited_by, int32 join_timestamp, int8 rank);`
- `void sendPeersRemoveGuildMember(int32 character_id, int32 guild_id, std::string removed_by);`
- `void sendPeersCreateGuild(int32 guild_id);`
- `void sendPeersGuildPermission(int32 guild_id, int8 rank, int8 permission, int8 value_);`
- `void sendPeersGuildEventFilter(int32 guild_id, int8 event_id, int8 category, int8 value_);`
- `void SetPeerErrorState(std::string address, std::string port);`
- `void handlePrimaryConflict(const std::string& reconnectingPeerId);`
- `std::shared_ptr<Peer> getCurrentPrimary();`
- `void sendPeersMessage(const std::string& endpoint, int32 command, int32 sub_command = 0);`
- `void sendZonePlayerList(std::vector<string>* queries, std::vector<WhoAllPeerPlayer>* peer_list, bool isGM);`
- `bool GetClientGuildDetails(int32 matchCharID, GuildMember* member_details);`

## Notable Comments

- /*
- */
- // Default constructor
- // Example function to output data as JSON string (for debug or logging)
- // Example function to output data as JSON string (for debug or logging)
