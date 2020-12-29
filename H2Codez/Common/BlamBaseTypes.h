#pragma once
#include "../stdafx.h"

#define NONE -1

struct datum
{
	short index = NONE;
	short salt = NONE;

	datum(size_t info)
	{
		index = LOWORD(info);
		salt = HIWORD(info);
	}

	constexpr datum()
	{
	}

	constexpr datum(short _index, short _salt):
		index(_index),
		salt(_salt)
	{
	}

	/* Checks if the datum seems valid */
	constexpr bool is_valid() const
	{
		return (index != NONE) && (salt != NONE);
	}

	constexpr long as_long() const
	{
		return MAKELONG(index, salt);
	}

	constexpr static datum null()
	{
		return datum(NONE, NONE);
	}

	constexpr bool operator==(const datum &other) const
	{
		return this->as_long() == other.as_long();
	}

	constexpr bool operator!=(const datum &other) const
	{
		return !operator==(other);
	}

	constexpr bool operator!() const
	{
		return !is_valid();
	}

	void clear() 
	{
		index = NONE;
		salt = NONE;
	}
};
CHECK_STRUCT_SIZE(datum, 4);

/* Helper type for the multi-character constants often used in blam */
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

	inline std::string as_string() const
	{
		if (is_none())
			return "NONE";
		if (is_null())
			return "";
		std::string out;
		out += c_data[3];
		out += c_data[2];
		out += c_data[1];
		out += c_data[0];
		return out;
	}

	constexpr int as_int() const
	{
		return i_data;
	}

	constexpr bool is_null() const
	{
		return as_int() == NULL;
	}

	constexpr bool is_none() const
	{
		return as_int() == NONE;
	}

	constexpr bool is_set() const
	{
		return !is_null() && !is_none();
	}

	constexpr bool is_printable() const
	{
		return isprint(c_data[0]) && isprint(c_data[1]) && isprint(c_data[2]) && isprint(c_data[3]);
	}

	constexpr bool operator==(const blam_tag &other) const
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

	constexpr editor_string() = default;

	constexpr editor_string(const char* _string) :
		string(_string)
	{
	}
	
	constexpr editor_string(size_t _id) :
		id(_id)
	{
	}

	bool is_string_id()
	{
		// assume it's a c-string if it's less than a hardcoded max id
		return id <= max_string_id;
	}

#pragma optimize( "", off )
	// really nasty code, needed because of some invalid data in tag defs
	bool is_string_ptr_valid(size_t &size_hack)
	{
		if (is_string_id())
			return false;
		__try
		{
			size_hack = strlen(string);
			return true;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}
#pragma optimize( "", on )

	// is string not set or empty
	bool is_empty()
	{
		return id == empty_string_id || string == NULL;
	}

	// returns contents as C++ string
	std::string get_string()
	{
		if (is_empty())
			return "";
		if (!is_string_id()) {
			size_t size;
			if (LOG_CHECK(is_string_ptr_valid(size)))
				return string;
			else
				return "INVALID_STRING";
		}
		char data[0x1000];
		if (!LOG_CHECK(LoadStringA(get_h2alang(), id, data, ARRAYSIZE(data))))
		{
			getLogger().WriteLog("Failed to get string %d", id);
			return "BORK BORK BORK";
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

struct colour_argb
{
	size_t data;
};

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

	real_point3d operator+(const real_point3d& other) const {
		return real_point3d{ this->x + other.x, this->y + other.y, this->z + other.z };
	}
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

struct real_quaternion
{
	float i;
	float j;
	float k;
	float w;

	inline float get_square_length() const
	{
		return i * i + j * j + k * k + w * w;
	}

	static inline real_quaternion from_angle(real_euler_angles3d angle) {
		double cy = cos(angle.yaw.as_rad() * 0.5);
		double sy = sin(angle.yaw.as_rad() * 0.5);
		double cp = cos(angle.pitch.as_rad() * 0.5);
		double sp = sin(angle.pitch.as_rad() * 0.5);
		double cr = cos(angle.roll.as_rad() * 0.5);
		double sr = sin(angle.roll.as_rad() * 0.5);

		real_quaternion q;
		q.w = static_cast<float>(cr * cp * cy + sr * sp * sy);
		q.i = static_cast<float>(sr * cp * cy - cr * sp * sy);
		q.j = static_cast<float>(cr * sp * cy + sr * cp * sy);
		q.k = static_cast<float>(cr * cp * sy - sr * sp * cy);

		return q;
	}
};

struct real_bounds
{
	float lower;
	float upper;
};

struct short_bounds
{
	short lower;
	short upper;
};

struct angle_bounds
{
	angle lower;
	angle upper;
};

struct string_id
{
	string_id() = default;

	constexpr string_id(uint32_t _value) :
		value(_value)
	{};

	constexpr string_id(uint32_t id, uint8_t length) :
		value(id | (length << 24))
	{
	};

	constexpr uint8_t get_length() const
	{
		return (value >> 24) & 0xFFu;
	}

	constexpr uint32_t get_id() const
	{
		return value & ~(0xffu << 24);
	}

	constexpr uint32_t get_packed() const
	{
		return value;
	}

	constexpr bool is_valid() const
	{
		return get_packed() != 0;
	}

	const char* get_name() const
	{
		auto name = get_string_name_table()[get_id()];
		if (is_debug_build())
			LOG_CHECK(strnlen_s(name, 0x60000) == get_length());
		return name;
	}

	/* Attempts to find a string with that name in the hash table */
	static inline string_id find_by_name(const char *string)
	{
		typedef uint32_t(__cdecl *get_string_id)(const char *string);
		get_string_id get_string_id_impl = reinterpret_cast<get_string_id>(SwitchAddessByMode(0x0052E830, 0x004B0200, 0));
		CHECK_FUNCTION_SUPPORT(get_string_id_impl);

		return get_string_id_impl(string);
	}

	/* Attempts to find a string with that name in the hash table */
	static inline string_id find_by_name(const std::string &string)
	{
		return find_by_name(string.c_str());
	}

	/* Look for a string id and registers it if the function can't find it */
	static inline string_id get_string_id(const char* string)
	{
		typedef uint32_t __cdecl get_string_id_by_name_or_register(const char* _name);
		auto *get_string_id_by_name_or_register_impl = reinterpret_cast<get_string_id_by_name_or_register*>(SwitchAddessByMode(0x52E8A0, 0, 0));
		CHECK_FUNCTION_SUPPORT(get_string_id_by_name_or_register_impl);

		return get_string_id_by_name_or_register_impl(string);
	}

	/* Look for a string id and registers it if the function can't find it */
	static inline string_id get_string_id(const std::string& string)
	{
		return get_string_id(string.c_str());
	}

	struct string_id_globals
	{
		struct {
			char* allocation;
			size_t next_offset;
		} name_memory;
		const char** names_table;
		long count;
		void* hashtable;
	};

	static inline string_id_globals* get_string_id_globals()
	{
		auto* globals = reinterpret_cast<string_id_globals*>(SwitchAddessByMode(0xA801E0, 0xA754C8, 0x9B7E60));
		CHECK_FUNCTION_SUPPORT(globals);
		return globals;
	}

	static inline long get_string_id_count()
	{
		return get_string_id_globals()->count;
	}

	static inline const char** get_string_name_table()
	{
		return get_string_id_globals()->names_table;
	}

private:
	uint32_t value;
};
CHECK_STRUCT_SIZE(string_id, 4);

typedef string_id old_string_id;

struct tag_enum_map_element
{
	editor_string string;
	DWORD number;
	constexpr tag_enum_map_element(editor_string _string, int _number) :
		string(_string),
		number(_number)
	{}
};

struct tag_enum_def
{
	size_t count;
	editor_string *names;
};

struct rect2d
{
	WORD top;
	WORD left;
	WORD bottom;
	WORD right;
};

struct real_matrix4x3
{
	float scale = 1.0f;
	real_vector3d forward = {1.f, 0.f, 0.f};
	real_vector3d left    = {0.f, 1.f, 0.f};
	real_vector3d up      = {0.f, 0.f, 1.f};
	real_point3d translation = {0.f, 0.f, 0.f};

	constexpr real_matrix4x3() = default;
	constexpr real_matrix4x3(real_vector3d _forward, real_vector3d _left, real_vector3d _up, real_point3d _translation = { 0.f, 0.f, 0.f }):
		forward(_forward),
		left(_left),
		up(_up),
		translation(_translation)
	{
	}

	real_matrix4x3(const real_quaternion& rotation)
	{
		set_rotation(rotation);
	}

	real_matrix4x3(const real_quaternion& _rotation, const real_point3d& _translation) :
		translation(_translation)
	{
		set_rotation(_rotation);
	}

	inline void inverse_rotation()
	{
		std::swap(forward.j, left.i);
		std::swap(forward.k, up.i);
		std::swap(left.k, up.j);
	}

	inline void inverse()
	{
		assert(scale != 0.0f);
		scale = 1.0f / scale;

		inverse_rotation();

		float inverse_pos_x = -translation.x * scale;
		float inverse_pos_y = -translation.y * scale;
		float inverse_pos_z = -translation.z * scale;

		translation.x = (inverse_pos_x * forward.i) + (inverse_pos_y * left.i) + (inverse_pos_z * up.i);
		translation.y = (inverse_pos_x * forward.j) + (inverse_pos_y * left.j) + (inverse_pos_z * up.j);
		translation.z = (inverse_pos_x * forward.k) + (inverse_pos_y * left.k) + (inverse_pos_z * up.k);
	};

	void set_rotation(const real_quaternion& rotation)
	{
		float square_len = rotation.get_square_length();
		assert(square_len != 0.0f);
		float s = 2.0f / square_len;

		auto is = rotation.i * s;
		auto js = rotation.j * s;
		auto ks = rotation.k * s;

		auto iw = rotation.w * is;
		auto jw = rotation.w * js;
		auto kw = rotation.w * ks;

		auto ii = rotation.i * is, jj = rotation.j * js, kk = rotation.k * ks;
		auto ij = rotation.i * js, ik = rotation.i * ks, jk = rotation.j * ks;

		forward = { 1.0f - (jj + kk),  ij - kw,            ik + jw };
		left    = { ij + kw,           1.0f - (ii + kk),   jk - iw };
		up      = { ik - jw,           jk + iw,            1.0f - (ii + jj) };
	}
};

static inline real_vector3d operator*(const real_matrix4x3& lhs, const real_vector3d& rhs) {
	auto c1 = lhs.forward.i * rhs.i + lhs.left.i * rhs.j + lhs.up.i * rhs.k;
	auto c2 = lhs.forward.j * rhs.i + lhs.left.j * rhs.j + lhs.up.j * rhs.k;
	auto c3 = lhs.forward.k * rhs.i + lhs.left.k * rhs.j + lhs.up.k * rhs.k;
	return real_vector3d{ c1, c2, c3 };
}

static inline real_point3d operator*(const real_matrix4x3& lhs, const real_point3d& rhs) {
	auto c1 = lhs.forward.i * rhs.x + lhs.left.i * rhs.y + lhs.up.i * rhs.z;
	auto c2 = lhs.forward.j * rhs.x + lhs.left.j * rhs.y + lhs.up.j * rhs.z;
	auto c3 = lhs.forward.k * rhs.x + lhs.left.k * rhs.y + lhs.up.k * rhs.z;
	return real_point3d{ c1, c2, c3 };
}

