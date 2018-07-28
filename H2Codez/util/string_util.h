#pragma once
#include <string>
#include <cctype>

inline std::string tolower(const std::string &str)
{
	std::string lower_case;
	for (unsigned char elem : str) {
		lower_case += static_cast<unsigned char>(std::tolower(elem));
	}
	return lower_case;
};

inline void str_trim(std::string &str, const char trim_char = ' ')
{
	if (str.empty())
		return;
	if (str.find_first_not_of(trim_char) == std::string::npos)
		str.clear();
	else
		str = str.substr(str.find_first_not_of(trim_char), str.find_last_not_of(trim_char) + 1);
}
