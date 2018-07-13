#pragma once
#include <stdint.h>

namespace TextRasterizer {

	struct text_rasterizer_screen_bounds
	{
		uint16_t y0;
		uint16_t x0;
		uint16_t y1;
		uint16_t x1;
	};
}