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

inline void str_trim(std::string &str)
{
	str = str.substr(str.find_first_not_of(' '), str.find_last_not_of(' ') + 1);
}
