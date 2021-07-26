#pragma once
#include <functional>
#include <variant>
#include <string>

class FastString
{
public:
	FastString() {
		contents = "";
	}
	/// <summary>
	/// Construct the string with a const string, it is your responsibility to ensure this string remains valid
	/// </summary>
	/// <param name="string"></param>
	FastString(const char* string) {
		contents = string;
	}

	FastString(const std::string& string) {
		contents = string;
	}

	inline const char* get() const {
		return std::holds_alternative<std::string>(contents) ?
			std::get<std::string>(contents).c_str() :
			std::get<const char*>(contents);
	}

	bool operator<(const FastString& other) const  {
		if (contents.index() == other.contents.index() && contents.index() == 1) {
			return std::get<std::string>(contents) < std::get<std::string>(other.contents);
		}

		return strcmp(get(), other.get()) < 0;
	}

	bool operator>(const FastString& other) const {
		if (contents.index() == other.contents.index() && contents.index() == 1) {
			return std::get<std::string>(contents) > std::get<std::string>(other.contents);
		}

		return strcmp(get(), other.get()) > 0;
	}

	bool operator==(const FastString& other) const {
		if (contents.index() == other.contents.index() && contents.index() == 1) {
			return std::get<std::string>(contents) == std::get<std::string>(other.contents);
		}

		return strcmp(get(), other.get()) == 0;
	}

private:
	std::variant<const char*, std::string> contents;
};
