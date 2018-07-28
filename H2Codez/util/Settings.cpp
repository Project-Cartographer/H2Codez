#include "Settings.h"
#include <fstream>
#include <cctype>
#include "string_util.h"

Settings::Settings(const std::string &settings_path) :
	settings_filename(settings_path)
{
	std::ifstream settings_file(settings_path);
	while (settings_file && !settings_file.eof()) {
		std::string line;
		std::getline(settings_file, line);

		size_t cut_point = line.find_first_of('=');
		if (cut_point == std::string::npos)
			continue;
		std::string setting = line.substr(0, cut_point);
		std::string value = line.substr(cut_point + 1);

		str_trim(setting);
		str_trim(value);
		if (validate_setting_name(setting))
			key_value_pairs[setting] = value;
	}
	settings_file.close();
}

Settings::~Settings()
{
	if (!settings_edited)
		return;
	std::ofstream settings_file(settings_filename);
	for (auto i : key_value_pairs)
		settings_file << i.first << " = " << i.second << std::endl;
	settings_file.close();
}

const std::string& Settings::getString(const std::string &setting)
{
	if (validate_setting_name(setting)) {
		auto ilter = key_value_pairs.find(setting);
		if (ilter != key_value_pairs.end())
			return ilter->second;
		else
			throw SettingError("No such string");
	} else {
		throw SettingError("Invalid setting name");
	}
}

bool Settings::getString(const std::string &setting, std::string &value)
{
	if (validate_setting_name(setting)) {
		auto ilter = key_value_pairs.find(setting);
		if (ilter != key_value_pairs.end()) {
			value = ilter->second;
			return true;
		} else {
			return false;
		}
	}
	throw SettingError("Invalid setting name");
}

void Settings::setString(const std::string &setting, const std::string &value)
{
	if (validate_setting_name(setting)) {
		if (key_value_pairs[setting] != value) {
			key_value_pairs[setting] = value;
			settings_edited = true;
		}
	}
	else {
		throw SettingError("Invalid setting name");
	}
}

bool Settings::getBoolean(const std::string &setting, bool default)
{
	std::string value;
	if (!getString(setting, value) || value.empty()) {
		setBoolean(setting, default);
		return default;
	}
	auto str = tolower(value);
	try {
		return stol(value);
	}
	catch (std::invalid_argument) {
	}
	catch (std::out_of_range) {
	}
	if (str == "true" || str == "on") {
		return true;
	} else if (str == "false" || str == "off") {
		return false;
	} else {
		setBoolean(setting, default);
		return default;
	}
}

void Settings::setBoolean(const std::string &setting, bool value)
{
	setString(setting, value ? "true" : "false");
}
