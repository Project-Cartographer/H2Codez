#pragma once
#include "string_util.h"

namespace numerical {

#define CHECK_NUMERICAL_TYPE(type)\
	static_assert(std::is_arithmetic<type>::value, "NumericType must be numeric")

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

	/* Convert number to string */
	template <typename NumericType, bool uppercase = true>
	std::string to_string(NumericType number, radix base = decimal, size_t width = 0)
	{
		CHECK_NUMERICAL_TYPE(NumericType);
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

	/* Does real modulo */
	template <typename NumericType>
	inline int real_modulo(NumericType a, NumericType b) {
		CHECK_NUMERICAL_TYPE(NumericType);

		if (b < 0) return real_modulo(-a, -b);
		const int result = a % b;
		return result >= 0 ? result : result + b;
	}

	/* range limits a value between min and max */
	template <typename NumericType>
	inline int range_limit(NumericType value, NumericType min, NumericType max) {
		CHECK_NUMERICAL_TYPE(NumericType);
		if (min > max)
			return range_limit<NumericType>(value, max, min);

		return std::max(min, std::min(value, max));
	}

	/* returns true if a v is between lo and hi */
	template<typename T1, typename T2, typename T3>
	constexpr bool is_between(const T1& v, const T2& lo, const T3& hi)
	{
		return (v >= lo) && (v <= hi);
	}


}
