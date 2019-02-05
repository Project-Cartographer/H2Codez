#pragma once
#include "string_util.h"

namespace numerical_util {

	enum radix
	{
		octal = 8,
		decimal = 10,
		hexadecimal = 16
	};

	/* returns the base/radix of a number */
	inline radix get_base(const std::string &number)
	{
		std::string value = tolower(number);
		str_trim(value);
		if (!value.empty() && value[0] == '0')
		{
			if (value.size() >= 2 && value[1] == 'x')
				return hexadecimal;
			return octal;
		}
		return decimal;
	};

	template <typename NumericType, bool uppercase = true>
	std::string to_string(NumericType number, radix base = decimal, size_t width = 0)
	{
		static_assert(std::is_arithmetic<NumericType>::value, "NumericType must be numeric");
		std::stringstream stream;

		switch (base)
		{
		case octal:
			stream << "0" << std::oct << std::setw(width) << std::uppercase << number;
			break;
		case hexadecimal:
			stream << "0x" << std::hex << std::setw(width) << std::uppercase << number;
			break;
		case decimal:
		default:
			stream << std::dec << number;
			break;
		}
		return stream.str();
	}

	template <typename NumericType>
	inline int real_modulo(NumericType a, NumericType b) {
		static_assert(std::is_arithmetic<NumericType>::value, "NumericType must be numeric");

		if (b < 0) return real_modulo(-a, -b);
		const int result = a % b;
		return result >= 0 ? result : result + b;
	}

	template<class T>
	constexpr bool is_between(const T& v, const T& lo, const T& hi)
	{
		return (v >= lo) && (v <= hi);
	}
}
