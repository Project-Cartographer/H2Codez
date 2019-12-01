#pragma once
#include "string_util.h"

namespace numerical {

#define CHECK_NUMERICAL_TYPE(type)\
	static_assert(std::is_arithmetic<type>::value, "NumericType must be numeric")

#define CHECK_INTEGRAL_TYPE(type)\
	static_assert(std::is_integral<type>::value, "IntegralType must be integral")

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

	/* Is a number a power of two? */
	template <typename IntegralType>
	inline constexpr bool is_power_of_two(IntegralType num)
	{
		CHECK_INTEGRAL_TYPE(IntegralType);
		return (num != 0) && ((num & (num - 1)) == 0);
	}

	/* 
		Returns an approximate solution for log2(num), always rounding down
		e.g. integral_log2(1023) == 9 and integral_log2(1025) == 10
	*/
	template <typename IntegralType>
	inline int integral_log2(IntegralType num)
	{
		CHECK_INTEGRAL_TYPE(IntegralType);
		int exponent = 0;
		while (num >>= 1)
			exponent++;
		return exponent;
	}


	template <typename NumericType>
	inline NumericType div(NumericType a, NumericType num) {
		CHECK_NUMERICAL_TYPE(NumericType);
		return a / num;
	}

	template <typename NumericType>
	inline NumericType mul(NumericType a, NumericType b) {
		CHECK_NUMERICAL_TYPE(NumericType);
		return a * b;
	}

	template <typename NumericType>
	inline NumericType sub(NumericType a, NumericType num) {
		CHECK_NUMERICAL_TYPE(NumericType);
		return a - num;
	}

	template <typename NumericType>
	inline NumericType add(NumericType a, NumericType b) {
		CHECK_NUMERICAL_TYPE(NumericType);
		return a + b;
	}

#undef CHECK_NUMERICAL_TYPE
}
