
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <string>
#include <map>

class JsonParser {
public:
    JsonParser(const std::string &filename);

	std::string getValue(const std::string &path) const {
		auto it = values.find(path);
		if (it != values.end()) {
			return it->second;
		}
		return "";
	}
	
	static bool convertStringToUnsignedChar(const std::string& str, unsigned char& result);
	static bool convertStringToUnsignedShort(const std::string& str, unsigned short& result);
	static bool convertStringToUnsignedInt(const std::string& str, unsigned int& result);
	static bool convertStringToUnsignedLong(const std::string& str, unsigned long& result);
	bool IsLoaded() { return is_loaded; }
private:
    boost::property_tree::ptree pt;
    std::map<std::string, std::string> values;

    void parseTree(const boost::property_tree::ptree &tree, const std::string &path);
	bool is_loaded;
};
