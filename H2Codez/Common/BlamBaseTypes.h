#pragma once
#include "../stdafx.h"

struct colour_rgb;
/* channel intensity is represented on a 0 to 1 scale */
struct colour_rgba
{
	float alpha = 1.0f;
	float red = 1.0f;
	float green = 1.0f;
	float blue = 1.0f;

	colour_rgba() {}

	colour_rgba(float _alpha, float _red, float _green, float _blue) :
		alpha(_alpha),
		red(_red),
		green(_green),
		blue(_blue)
	{}
};

struct colour_rgb
{
	float red = 1.0f;
	float green = 1.0f;
	float blue = 1.0f;

	colour_rgb() {}

	colour_rgb(float _red, float _green, float _blue) :
		red(_red),
		green(_green),
		blue(_blue)
	{}

	colour_rgb(const colour_rgba &colour) :
		red(colour.red),
		green(colour.green),
		blue(colour.blue)
	{}

	colour_rgba as_rgba(float _alpha = 1.0f)
	{
		colour_rgba converted;
		converted.alpha = _alpha;
		converted.red = red;
		converted.green = green;
		converted.blue = blue;
		return converted;
	}
};

struct point2d
{
	short x;
	short y;
};
struct real_point2d
{
	float x;
	float y;
};

struct real_point3d
{
	float x;
	float y;
	float z;
};

struct real_vector2d
{
	float i;
	float j;
};

struct real_vector3d
{
	float i;
	float j;
	float k;
};

struct real_plane2d
{
	real_vector2d normal;
	float distance;
};

struct real_plane3d
{
	real_vector3d normal;
	float distance;
};

struct real_euler_angles2d
{
	float yaw;
	float pitch;
};

struct real_bounds
{
	float lower;
	float upper;
};


struct string_id
{
	DWORD id;
};

struct old_string_id
{
	DWORD id;
};

struct tag_enum_map_element
{
	DWORD string;
	DWORD number;
	tag_enum_map_element(char* _string, int _number) :
		string(reinterpret_cast<DWORD>(_string)),
		number(_number)
	{}
	tag_enum_map_element(DWORD _string, int _number) :
		string(_string),
		number(_number)
	{}

};
