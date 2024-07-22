#include "../net.h"
#include "../LWorld.h"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

extern ClientList client_list;
extern LWorldList world_list;
extern NetConnection net;

void NetConnection::Web_loginhandle_status(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    res.set(http::field::content_type, "application/json");
	boost::property_tree::ptree pt;
	
    pt.put("web_status", "online");
    pt.put("login_status", net.login_running ? "online" : "offline");
    pt.put("login_uptime", (getCurrentTimestamp() - net.login_uptime));
	auto [days, hours, minutes, seconds] = convertTimestampDuration((getCurrentTimestamp() - net.login_uptime));
	std::string uptime_str("Days: " + std::to_string(days) + ", " + "Hours: " + std::to_string(hours) + ", " + "Minutes: " + std::to_string(minutes) + ", " + "Seconds: " + std::to_string(seconds));
    pt.put("login_uptime_string", uptime_str);
    pt.put("world_count", world_list.GetCount(ConType::World));
    pt.put("client_count", net.numclients);

    std::ostringstream oss;
    boost::property_tree::write_json(oss, pt);
    std::string json = oss.str();
    res.body() = json;
    res.prepare_payload();
}

void NetConnection::Web_loginhandle_worlds(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	world_list.PopulateWorldList(res);
}

void LWorldList::PopulateWorldList(http::response<http::string_body>& res) {

	struct in_addr in;
    res.set(http::field::content_type, "application/json");
	boost::property_tree::ptree maintree;

    std::ostringstream oss;

	map<int32,LWorld*>::iterator map_list;
	for( map_list = worldmap.begin(); map_list != worldmap.end(); map_list++) {
		LWorld* world = map_list->second;
		in.s_addr = world->GetIP();
		if (world->GetType() == World) {
			
			boost::property_tree::ptree pt;
			pt.put("id", world->GetID());
			pt.put("world_name", world->GetName());
			pt.put("status", (world->GetStatus() == 1) ? "online" : "offline");
			pt.put("ip_addr", inet_ntoa(in));
			maintree.add_child("WorldServer", pt);
		}
	}
	
    boost::property_tree::write_json(oss, maintree);
    std::string json = oss.str();
    res.body() = json;
    res.prepare_payload();
}

