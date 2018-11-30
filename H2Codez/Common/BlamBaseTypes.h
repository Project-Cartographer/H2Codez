#pragma once
#include "../stdafx.h"

#define NONE -1

struct datum
{
	short index;
	short salt;
	datum(size_t info)
	{
		index = LOWORD(info);
		salt = HIWORD(info);
	}
	bool is_valid()
	{
		return (index != NONE) && (salt != NONE);
	}

	long as_long()
	{
		return *reinterpret_cast<long*>(this);
	}
};
CHECK_STRUCT_SIZE(datum, 4);

struct blam_tag
{
	union {
		char c_data[4];
		uint32_t i_data ;
	};

	blam_tag()
	{
	}

	constexpr blam_tag(uint32_t data) :
		i_data(data)
	{
	}

	std::string as_string() const
	{
		std::string out;
		out += c_data[3];
		out += c_data[2];
		out += c_data[1];
		out += c_data[0];
		return out;
	}

	int as_int() const
	{
		return i_data;
	}

	bool operator==(const blam_tag &other) const
	{
		return this->as_int() == other.as_int();
	}

	constexpr static blam_tag null()
	{
		return blam_tag(0xFFFFFFFF);
	}
};

struct editor_string
{
	constexpr static size_t max_string_id = 5207; // highest id for a string in h2alang
	constexpr static size_t empty_string_id = 87; // id for empty string

	union {
		const char *string;
		size_t id;
	};

	constexpr editor_string(const char* _string) :
		string(_string)
	{
	}
	
	constexpr editor_string(size_t _id) :
		id(_id)
	{
	}

	std::string get_string()
	{
		if (id > max_string_id) // assume it's a c-string
			return string;
		if (id == empty_string_id) // empty string
			return "";
		char data[0x1000];
		if (!LOG_CHECK(LoadStringA(get_h2alang(), id, data, ARRAYSIZE(data))))
		{
			pLog.WriteLog("Failed to get string %d", id);
			return "";
		}
		return data;
	}

	// h2alang util
	static HMODULE get_h2alang()
	{
		static HMODULE handle = NULL;
		if (!handle)
			handle = LOG_CHECK(LoadLibraryExA("h2alang.dll", NULL, LOAD_LIBRARY_AS_DATAFILE));
		return handle;
	}
};
CHECK_STRUCT_SIZE(editor_string, 4);

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
	void clamp()
	{
		clamp_value(alpha);
		clamp_value(red);
		clamp_value(green);
		clamp_value(blue);
	}
private :
	void clamp_value(float &value)
	{
		value = std::min(std::max(0.0f, value), 1.0f);
	}
};
CHECK_STRUCT_SIZE(colour_rgba, 4 * 4);

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
CHECK_STRUCT_SIZE(colour_rgb, 4 * 3);

struct point2d
{
	short x;
	short y;
};
CHECK_STRUCT_SIZE(point2d, 2 * 2);

struct real_point2d
{
	float x;
	float y;
};
CHECK_STRUCT_SIZE(real_point2d, 4 * 2);

struct real_point3d
{
	float x;
	float y;
	float z;
};
CHECK_STRUCT_SIZE(real_point3d, 4 * 3);

struct angle
{
	float rad = 0.0f;

	angle() {};

	angle(float _rad) :
		rad(_rad)
	{}

	double as_degree()
	{
		return rad * (180.0 / 3.14159265358979323846);
	}

	double as_rad()
	{
		return rad;
	}
};
CHECK_STRUCT_SIZE(angle, sizeof(float));
CHECK_STRUCT_SIZE(angle, 4);

struct real_euler_angles2d
{
	angle yaw;
	angle pitch;
};
CHECK_STRUCT_SIZE(real_euler_angles2d, 4 * 2);

struct real_euler_angles3d
{
	angle yaw;
	angle pitch;
	angle roll;
};
CHECK_STRUCT_SIZE(real_euler_angles3d, 4 * 3);

struct real_vector2d
{
	float i;
	float j;
};
CHECK_STRUCT_SIZE(real_vector2d, 4 * 2);

struct real_vector3d
{
	float i;
	float j;
	float k;
	real_euler_angles3d get_angle()
	{
		real_euler_angles3d angle;
		angle.yaw = acos(i);
		angle.pitch = acos(j);
		angle.roll = acos(k);
		return angle;
	}
};
CHECK_STRUCT_SIZE(real_vector3d, 4 * 3);

struct real_plane2d
{
	real_vector2d normal;
	float distance;
};
CHECK_STRUCT_SIZE(real_plane2d, sizeof(real_vector2d) + 4);

struct real_plane3d
{
	real_vector3d normal;
	float distance;
};
CHECK_STRUCT_SIZE(real_plane3d, sizeof(real_vector3d) + 4);

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
	editor_string string;
	DWORD number;
	constexpr tag_enum_map_element(const char* _string, int _number) :
		string(_string),
		number(_number)
	{}
	constexpr tag_enum_map_element(DWORD _string, int _number) :
		string(_string),
		number(_number)
	{}

};
