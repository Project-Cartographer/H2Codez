#pragma once
#include <string>
#include <cctype>
#include <sstream>
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

/*
	Returns true if a string is a decimal number
*/
inline bool is_string_numerical(const std::string &str)
{
	if (str.size() == 0)
		return false;
	for (unsigned char elem : str) {
		if (!isdigit(elem))
			return false;
	}
	return true;
}

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

template<typename T>
inline std::string as_hex_string(const T *element_array, size_t element_count)
{
	const char *data = reinterpret_cast<const char*>(element_array);
	std::stringstream ss;
	ss << std::setfill('0') << std::hex;
	for (size_t i = 0; i < element_count * sizeof(T); i++)
	{
		ss << std::setw(2) << static_cast<int>(data[i]);
	}
	return ss.str();
}

template<>
inline std::string as_hex_string(const void *element_array, size_t element_count)
{
	return as_hex_string(reinterpret_cast<const char*>(element_array), element_count);
}

inline std::string sanitize_filename(std::string name) {
	constexpr char chars_to_replace[] = { '*', '?', '/', '\\', ':', '"', '|', '>', '<' };
	for (char replace : chars_to_replace)
		std::replace(name.begin(), name.end(), replace, '_');
	return name;
};


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

struct file_info {
	bool has_entension;
	std::string file_path;
	std::string extension = "";
};

inline file_info get_file_path_info(const std::string &path)
{
	file_info info;
	auto cut_point = path.find_last_of('.');
	if (cut_point == std::string::npos || (cut_point + 1) > path.size()) {
		info.has_entension = false;
		info.file_path = path;
	} else {
		info.has_entension = true;
		info.file_path = path.substr(0, cut_point);
		info.extension = path.substr(cut_point + 1);
	}

	return info;
}

// get full path to a tag path
std::string get_full_tag_path(const std::string &tag_path);

// file reference support code
constexpr size_t file_refernece_max_path_len = 0x100;
inline void append_name_to_path(char *path, const char *append)
{
	if (!LOG_CHECK(append) || *append == '\0')
		return;
	
	size_t path_len = strnlen_s(path, file_refernece_max_path_len);
	size_t append_len = strnlen_s(append, file_refernece_max_path_len);
	bool is_terminated_correctly = path[path_len] == '\\' || path_len == 0;

	size_t size_required = append_len + 1; // size to append + null terminator
	// if we need to add the path separator that will require an additional character
	if (is_terminated_correctly)
		size_required += path_len;
	else
		size_required += path_len + 1;

	ASSERT_CHECK(size_required <= file_refernece_max_path_len);

	if (!is_terminated_correctly)
	{
		path[path_len++] = '\\';
		path[path_len] = '\0';
	}

	strncat_s(path, sizeof(char) * file_refernece_max_path_len, append, append_len);
}

inline void remove_last_part_of_path(char *path)
{
	ASSERT_CHECK(path);
	for (size_t string_offset = strnlen_s(path, file_refernece_max_path_len) - 1;
		string_offset >= 0; string_offset--)
	{
		char current_char = path[string_offset];
		path[string_offset] = '\0';
		if (current_char == '\\')
			break;
	}
}

inline size_t strrchr_offset(const char *string, int c)
{
	ASSERT_CHECK(string);
	const char *offset_string = strrchr(string, c);
	if (offset_string == nullptr)
		return NONE;
	else
		return (offset_string - string);
}

inline std::string duplicate_last_path_element(std::string path)
{
	auto offset = path.find_last_of('\\');
	if (offset == std::string::npos)
		return path + "\\" + path;
	if (offset == path.size() - 1) // this is nasty...
		return duplicate_last_path_element(path.substr(0, offset));
	std::string last_element = path.substr(offset);
	return path + last_element;
}

inline std::string tag_path_from_import_path(const std::string &import_path)
{
	std::string path_lowercase = tolower(import_path);
	static constexpr char data[] = "data\\";
	const char *path_start = strstr(path_lowercase.c_str(), data);
	if (path_start) {
		path_start += strlen(data);
	} else {
		path_start = path_lowercase.c_str();
	}

	size_t ext_offset = strrchr_offset(path_start, '.');
	std::string output_path;
	if (ext_offset == NONE)
		output_path = std::string(path_start);
	else
		output_path = std::string(path_start, ext_offset);
	return duplicate_last_path_element(output_path);
}
