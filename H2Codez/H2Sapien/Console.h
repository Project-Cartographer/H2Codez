#pragma once
#include "../Common/BlamBaseTypes.h"
#include <string>

namespace H2SapienConsole
{
	inline void print(const std::string &message, const colour_rgba text_colour = colour_rgba())
	{
		typedef char(*print_to_screen_with_colour)(const colour_rgba *colours, char *Format, ...);
		auto print_to_screen_with_colour_impl = reinterpret_cast<print_to_screen_with_colour>(0x00504BC0);
		print_to_screen_with_colour_impl(&text_colour, "%s", message.c_str());
	};

	void run_hs_command(const std::string &script);
};

namespace H2SapienPatches {
	void ConsoleInit();
};
