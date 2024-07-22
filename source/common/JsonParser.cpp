#include "JsonParser.h"

JsonParser::JsonParser(const std::string &filename) {
	is_loaded = false;
	try {
		boost::property_tree::read_json(filename, pt);
		parseTree(pt, "");
		is_loaded = true;
	} catch (const boost::property_tree::json_parser_error &e) {
		std::cerr << "Error reading JSON file: " << e.what() << std::endl;
	}
}

bool JsonParser::convertStringToUnsignedChar(const std::string& str, unsigned char& result) {
	unsigned long ul;
	try {
		ul = std::stoul(str);
	} catch (const std::invalid_argument&) {
		return false; // Not a valid number
	} catch (const std::out_of_range&) {
		return false; // Number is too large for unsigned long
	}
	
	if (ul > std::numeric_limits<unsigned char>::max()) {
		return false; // Number is too large for unsigned short
	}

	result = static_cast<unsigned char>(ul);
	return true;
}

bool JsonParser::convertStringToUnsignedShort(const std::string& str, unsigned short& result) {
	unsigned long ul;
	try {
		ul = std::stoul(str);
	} catch (const std::invalid_argument&) {
		return false; // Not a valid number
	} catch (const std::out_of_range&) {
		return false; // Number is too large for unsigned long
	}
	
	if (ul > std::numeric_limits<unsigned short>::max()) {
		return false; // Number is too large for unsigned short
	}

	result = static_cast<unsigned short>(ul);
	return true;
}

bool JsonParser::convertStringToUnsignedInt(const std::string& str, unsigned int& result) {
	unsigned long ul;
	try {
		ul = std::stoul(str);
	} catch (const std::invalid_argument&) {
		return false; // Not a valid number
	} catch (const std::out_of_range&) {
		return false; // Number is too large for unsigned long
	}
	
	if (ul > std::numeric_limits<unsigned int>::max()) {
		return false; // Number is too large for unsigned short
	}

	result = static_cast<unsigned int>(ul);
	return true;
}

bool JsonParser::convertStringToUnsignedLong(const std::string& str, unsigned long& result) {
	unsigned long ul;
	try {
		ul = std::stoul(str);
	} catch (const std::invalid_argument&) {
		return false; // Not a valid number
	} catch (const std::out_of_range&) {
		return false; // Number is too large for unsigned long
	}
	
	if (ul > std::numeric_limits<unsigned long>::max()) {
		return false; // Number is too large for unsigned short
	}

	result = ul;
	return true;
}

void JsonParser::parseTree(const boost::property_tree::ptree &tree, const std::string &path) {
	for (const auto &node : tree) {
		std::string currentPath = path.empty() ? node.first : path + "." + node.first;
		if (node.second.empty()) {
			std::string name = currentPath;
			boost::algorithm::to_lower(name);
			values[name] = node.second.get_value<std::string>();
		} else {
			parseTree(node.second, currentPath);
		}
	}
}