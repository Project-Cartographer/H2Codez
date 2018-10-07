#pragma once
#include <map>
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
		return validate_setting_name(setting) &&
			(key_value_pairs.find(setting) != key_value_pairs.end() ||
			temp_setting_data.find(setting) != temp_setting_data.end());
	}
	
	/* Checks if the string is a valid setting name */
	inline bool is_setting_name_valid(const std::string &name)
	{
		return validate_setting_name(name);
	}

	/* Throws an error if the setting name is invalid */
	template <typename NumericType>
	NumericType getNumber(const std::string &setting, NumericType default_value)
	{
		static_assert(std::is_arithmetic<NumericType>::value, "NumericType must be numeric");
		std::string value;
		if (!getString(setting, value)) {
			setNumber(setting, default_value);
			return default_value;
		}
		try {
			if (std::is_integral<NumericType>::value) {
				if (std::is_signed<NumericType>::value)
					return static_cast<NumericType>(std::stoll(value));
				else
					return static_cast<NumericType>(std::stoull(value));
			}
			else if (std::is_floating_point<NumericType>::value) {
				return static_cast<NumericType>(std::stold(value));
			}
			else {
				throw std::runtime_error("Settings::getNumber: unknown NumericType");
			}
		}
		catch (std::invalid_argument) {
			setNumber(setting, default_value);
			return default_value;
		}
		catch (std::out_of_range) {
			setNumber(setting, default_value);
			return default_value;
		}
	}

	/* Throws an error if the setting name is invalid */
	template <typename NumericType>
	inline void setNumber(const std::string & setting, NumericType value)
	{
		static_assert(std::is_arithmetic<NumericType>::value, "NumericType must be numeric");
		setString(setting, std::to_string(value));
	}

	/* Throws an error if the setting name is invalid */
	bool getBoolean(const std::string &setting, bool default = false);
	/* Throws an error if the setting name is invalid */
	void setBoolean(const std::string & setting, bool value);

	/* set temp setting, returns success */
	inline bool setTempSetting(std::string setting, const std::string &value)
	{
		if (validate_setting_name(setting)) {
			temp_setting_data[setting] = value;
			return true;
		} else {
			return false;
		}
	}
private:
	/* check if the setting name is valid */
	inline bool validate_setting_name(const std::string &setting)
	{
		return !setting.empty() && isalpha(setting[0])
			&& std::find_if(setting.begin(), setting.end(), [](char c) {return !isalnum(c) && c != '_' && c != '-';  }) == setting.end();
	}
	// for the current session only, are not saved
	std::map<std::string, std::string> temp_setting_data;
	
	std::string settings_filename;
	std::map<std::string, std::string> key_value_pairs;
	bool settings_edited = false;
};

