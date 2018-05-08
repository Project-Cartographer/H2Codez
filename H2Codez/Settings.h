#pragma once
#include <unordered_map>
#include <string>
#include <algorithm> 

class SettingError : public std::runtime_error
{
private:
	const std::string error_msg;
public:
	SettingError(const std::string &error) : std::runtime_error(error),
		error_msg(error)
	{
	};
	const char* what()
	{
		return error_msg.c_str();
	}
};

class Settings
{
public:
	Settings(const std::string &settings_path);
	~Settings();

	/// string getters
	/* Throws an expection in case of an error */
	const std::string &getString(const std::string &setting);
	/* Returns success and throws and expection if the name is invalid */
	bool getString(const std::string &setting, std::string &value);

	/// string setters

	/* Throws an error if the setting name is invalid */
	void setString(const std::string & setting, const std::string & value);

	/// Util function

	/* Returns if a setting exists */
	inline bool exists(const std::string &setting)
	{
		return validate_setting_name(setting) && key_value_pairs.find(setting) != key_value_pairs.end();
	}

	/* Checks if the string is a valid setting name */
	inline bool is_setting_name_valid(const std::string &name)
	{
		return validate_setting_name(name);
	}

	/* Throws an error if the setting name is invalid */
	long getNumber(const std::string &setting, long default = 0);
	/* Throws an error if the setting name is invalid */
	void setNumber(const std::string & setting, long value);
	/* Throws an error if the setting name is invalid */
	bool getBoolean(const std::string &setting, bool default = false);
	/* Throws an error if the setting name is invalid */
	void setBoolean(const std::string & setting, bool value);
private:
	/* check if the setting name is valid */
	inline bool validate_setting_name(const std::string &setting)
	{
		return !setting.empty() && isalpha(setting[0])
			&& std::find_if(setting.begin(), setting.end(), [](char c) {return !isalnum(c) && c != '_' && c != '-';  }) == setting.end();
	}
	std::string settings_filename;
	std::unordered_map<std::string, std::string> key_value_pairs;
};

