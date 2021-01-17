#pragma once
#include <array>
#include <cstdint>

class crc32
{
public:
	// crc result
	struct result
	{
		std::uint32_t data = 0xFFFFFFFFu;
		result() {}

		template<typename T>
		result(T crc) : data(crc)
		{
			static_assert(std::is_arithmetic<T>::value, "T must be numeric");
		}

		bool operator==(result other)
		{
			return this->data == other.data;
		}

		template<typename T>
		bool operator==(T other)
		{
			static_assert(std::is_arithmetic<T>::value, "T must be numeric");
			return this->data == other;
		}
	};

	// polynomial used by halo
	static const std::uint_fast32_t default_polynomial = 0xEDB88320uL;

	/* 
		Calulates crc32 polynomial 
		`output` - initial checksum state
		`data` - data to be checksumed
		`size` - size of data in bytes
	*/
	template <std::uint_fast32_t reversed_polynomial = default_polynomial, typename t>
	static void calculate(result &output, const t *data, size_t size)
	{
		do_calculation(output, reinterpret_cast<const unsigned char *>(data), size);
	}

	/*
		Calulates crc32 polynomial, returns checksum
		`data` - data to be checksumed
		`size` - size of data in bytes
	*/
	template <std::uint_fast32_t reversed_polynomial = default_polynomial, typename t>
	static result calculate(const t *data, size_t size)
	{
		result output;
		calculate(output, data, size);
		return output;
	}


	/* 
		Calulates crc32 polynomial 
		`output` - initial checksum state
		`data` - data to be checksumed
	*/
	template <std::uint_fast32_t reversed_polynomial = default_polynomial, typename t, size_t size>
	static void calculate(result &output, const t(*data)[size])
	{
		calculate(output, data, size);
	}

	/*
		Calulates crc32 polynomial, returns checksum
		`data` - data to be checksumed
	*/
	template <std::uint_fast32_t reversed_polynomial = default_polynomial, typename t, size_t size>
	static result calculate(const t(*data)[size])
	{
		return calculate(data, size);
	}

private:
	// helper functions 

	/* Generates a lookup table for a given polynomial */
	template <std::uint_fast32_t reversed_polynomial>
	static inline std::array<std::uint_fast32_t, 256> generate_crc_lookup_table()
	{
		using std::uint_fast32_t;
		std::array<std::uint_fast32_t, 256> table;
		for (uint_fast32_t i = 0; i < table.max_size(); i++)
		{
			auto table_value = i;
			for (uint_fast32_t j = 0; j < 8; j++) {
				if (table_value & 1) {
					table_value >>= 1;
					table_value ^= reversed_polynomial;
				}
				else
					table_value >>= 1;
			}
			table[i] = table_value;
		}
		return table;
	}

	/* Implementation function to avoid recalculating the lookup table */
	template <std::uint_fast32_t reversed_polynomial = default_polynomial>
	static void do_calculation(result &output, const unsigned char *data, size_t size)
	{
		const static std::array<std::uint_fast32_t, 256> lookup_table = generate_crc_lookup_table<reversed_polynomial>();

		for (size_t i = 0; i < size; i++)
			output.data = (output.data >> 8) ^ lookup_table[(output.data & 0xff) ^ data[i]];
	}
};
