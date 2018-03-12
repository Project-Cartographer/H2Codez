#include "Settings.h"
#include <fstream>
#include <cctype>

std::string tolower(const std::string &str)
{
	std::string lower_case;
	for (char elem : str) {
		lower_case += static_cast<unsigned char>(std::tolower(elem));
	}
	return lower_case;
}

void trim(std::string &str)
{
	str = str.substr(str.find_first_not_of(' '), str.find_last_not_of(' '));
}

Settings::Settings(std::string settings_path)
{
	std::ifstream settings_file(settings_path);
	while (!settings_file.eof()) {
		std::string line;
		std::getline(settings_file, line);

		size_t cut_point = line.find_first_of('=');
		std::string setting = line.substr(0, cut_point);
		std::string value = line.substr(cut_point + 1);

		trim(setting);
		trim(value);

		key_value_pairs[setting] = value;
	}
}

const std::string& Settings::getString(const std::string &setting)
{
	return key_value_pairs[setting];
}

long Settings::getNumber(const std::string &setting, int default)
{
	try {
		return stol(getString(setting));
	}
	catch (std::invalid_argument) {
		return default;
	}
	catch (std::out_of_range) {
		return default;
	}
}

bool Settings::getBoolean(const std::string &setting, bool default)
{
	auto str = tolower(getString(setting));
	auto num = getNumber(setting, static_cast<int>(default));
	if (str == "true" || str == "on" || num == 1) {
		return true;
	} else if (str == "false" || str == "off" || num == 0) {
		return true;
	} else {
		return default;
	}
}
