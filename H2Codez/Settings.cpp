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

Settings::Settings(const std::string &settings_path) :
	settings_filename(settings_path)
{
	std::ifstream settings_file(settings_path);
	while (!settings_file.badbit && !settings_file.eof()) {
		std::string line;
		std::getline(settings_file, line);

		size_t cut_point = line.find_first_of('=');
		if (cut_point == std::string::npos)
			continue;
		std::string setting = line.substr(0, cut_point);
		std::string value = line.substr(cut_point + 1);

		trim(setting);
		trim(value);
		if (validate_setting_name(setting))
			key_value_pairs[setting] = value;
	}
	settings_file.close();
}

Settings::~Settings()
{
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
		key_value_pairs[setting] = value;
	}
	else {
		throw SettingError("Invalid setting name");
	}
}

long Settings::getNumber(const std::string &setting, long default)
{
	std::string value;
	if (!getString(setting, value)) {
		setNumber(setting, default);
		return default;
	}
	try {
		return stol(value);
	}
	catch (std::invalid_argument) {
		setNumber(setting, default);
		return default;
	}
	catch (std::out_of_range) {
		setNumber(setting, default);
		return default;
	}
}

void Settings::setNumber(const std::string &setting, long value)
{
	setString(setting, std::to_string(value));
}

bool Settings::getBoolean(const std::string &setting, bool default)
{
	std::string value;
	if (!getString(setting, value) || value.empty()) {
		setBoolean(setting, default);
		return default;
	}
	auto str = tolower(value);
	auto num = getNumber(setting, static_cast<int>(default));
	if (str == "true" || str == "on" || num == 1) {
		return true;
	} else if (str == "false" || str == "off" || num == 0) {
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
