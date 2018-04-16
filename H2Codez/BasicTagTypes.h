#pragma once

#define NONE -1

struct tag_ref
{
	int tag_type;
	void *tag_pointer;
	int field_8;
	int tag_index;
};

struct tag_block_ref
{
	int size;
	void *data;
	void *definition;
};

template <typename T>
struct tag_block
{
	int size;
	T *data;
	void *definition;

	inline tag_block_ref *get_ref() const
	{
		return (tag_block_ref*)(this);
	}
};

struct byte_ref
{
	int size;
	int stream_flags;
	int stream_offset;
	void *address;
	int definition;
};

typedef unsigned int StringId;
typedef unsigned int OldStringId;
typedef float real;

struct point2d
{
	short x;
	short y;
};

struct rectangle2d
{
	short top;
	short left;
	short bottom;
	short right;
};

typedef unsigned int rgb_color;

typedef unsigned int argb_color;

struct short_bounds
{
	short lower;
	short upper;
};

typedef float real_fraction;

struct real_bounds
{
	real lower;
	real upper;
};

struct RealPoint2D
{
	real x;
	real y;
};

struct RealPoint3D
{
	real x;
	real y;
	real z;
};

struct real_vector2d
{
	real i;
	real j;
};

struct real_vector3d
{
	real i;
	real j;
	real k;
};

struct real_quaternion
{
	real i;
	real j;
	real k;
	real w;
};

struct real_plane2d
{
	real_vector2d normal;
	real distance;
};

struct real_plane3d
{
	real_vector3d normal;
	real distance;
};

struct real_rgb_color
{
	real red;
	real green;
	real blue;
};

struct real_argb_color
{
	real alpha;
	real red;
	real green;
	real blue;
};

struct real_hsv_color
{
	real hue;
	real saturation;
	real value;
};

struct real_ahsv_color
{
	real alpha;
	real hue;
	real saturation;
	real value;
};

typedef float angle;

struct angle_bounds
{
	angle lower;
	angle upper;
};

struct RealAngle2D
{
	angle yaw;
	angle pitch;
};

struct RealAngle3D
{
	angle yaw;
	angle pitch;
	angle roll;
};
