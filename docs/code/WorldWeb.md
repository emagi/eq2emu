# File: `WorldWeb.cpp`

## Classes

_None detected_

## Functions

- `void World::Web_worldhandle_status(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `void World::Web_populate_status(boost::property_tree::ptree& pt) {`
- `void World::Web_worldhandle_clients(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `void ZoneList::PopulateClientList(boost::property_tree::ptree& pt) {`
- `void World::Web_worldhandle_setadminstatus(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string charname("");`
- `else if (!got_status_field || charname.size() < 1) {`
- `void World::Web_worldhandle_reloadrules(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `void World::Web_worldhandle_reloadcommand(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `void World::Web_worldhandle_addpeer(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string client_addr("");`
- `std::string client_internal_addr("");`
- `std::string web_addr("");`
- `else if (got_addr)`
- `else if (got_port)`
- `void World::Web_worldhandle_zones(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `void ZoneList::PopulateZoneList(boost::property_tree::ptree& pt) {`
- `void World::Web_worldhandle_addcharauth(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string clientIP("");`
- `std::string charName(""), zoneName("");`
- `void World::Web_worldhandle_startzone(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string zoneName("");`
- `void World::Web_worldhandle_sendglobalmessage(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string toName(""), fromName(""), msg("");`
- `void World::Web_worldhandle_newgroup(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string leader(""), member("");`
- `std::string web_address("");`
- `std::string fieldName("group_id_");`
- `else if (net.is_primary) {`
- `else if (group_id) {`
- `void World::Web_worldhandle_addgroupmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `void World::Web_worldhandle_removegroupmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string name("");`
- `void World::Web_worldhandle_disbandgroup(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `void World::Web_worldhandle_createguild(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string newGuildName(""), leaderName("");`
- `else if (guildID) {`
- `void World::Web_worldhandle_addguildmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string invitedBy("");`
- `void World::Web_worldhandle_removeguildmember(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `std::string removedBy("");`
- `void World::Web_worldhandle_setguildpermission(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `void World::Web_worldhandle_setguildeventfilter(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`
- `std::istringstream json_stream(req.body());`
- `void World::Web_worldhandle_peerstatus(const http::request<http::string_body>& req, http::response<http::string_body>& res) {`

## Notable Comments

- /*
- */
- // failed to find zone requested by peer
