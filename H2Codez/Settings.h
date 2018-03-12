#pragma once
#include <unordered_map>
#include <string>

class Settings
{
public:
	Settings(std::string settings_path);

	const std::string &getString(const std::string &setting);

	long getNumber(const std::string &setting, int default = 0);
	bool getBoolean(const std::string &setting, bool default = false);
private:
	std::unordered_map<std::string, std::string> key_value_pairs;
};

