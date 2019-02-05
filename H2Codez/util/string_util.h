#pragma once
#include <string>
#include <cctype>
#include "Common/BlamBaseTypes.h"

inline std::string transform_string(const std::string &str, int(*transform)(int ch))
{
	std::string new_string;
	for (unsigned char elem : str) {
		new_string += static_cast<unsigned char>(transform(elem));
	}
	return new_string;
}

inline std::string tolower(const std::string &str)
{
	return transform_string(str, std::tolower);
};

inline std::string toupper(const std::string &str)
{
	return transform_string(str, std::toupper);
};

inline std::string &str_trim(std::string &str, const std::string &trim_char = " ")
{
	if (str.empty())
		return str;
	auto start = str.find_first_not_of(trim_char);
	if (start == std::string::npos)
		str.clear();
	else
		str = str.substr(start, str.find_last_not_of(trim_char) - start + 1);
	return str;
}

inline bool string_to_colour_rgb(std::string str, colour_rgb &colour_out)
{
	str_trim(str, "# ");

	if (!std::all_of(str.begin(), str.end(), ::isxdigit))
		return false;
	if (str.size() >= 3)
	{
		size_t digit_per_colour = str.size() >= 6 ? 2 : 1;
		size_t red_end = 1 * digit_per_colour;
		size_t green_end = 2 * digit_per_colour;
		size_t blue_end = 3 * digit_per_colour;
		colour_rgb out;
		try {
			auto hex_to_float_colour = [digit_per_colour](const std::string &hex) -> float {
				float out = stoi(hex, nullptr, 16) / (digit_per_colour == 2 ? 255.f : 15.f);
				return std::max(0.f, std::min(out, 1.f));
			};

			out.red = hex_to_float_colour(str.substr(0, red_end));
			out.green = hex_to_float_colour(str.substr(red_end, green_end));
			out.blue = hex_to_float_colour(str.substr(green_end, blue_end));
		}
		catch (std::out_of_range)
		{
			return false;
		}
		colour_out = out;
		return true;
	}
	return false;
}
