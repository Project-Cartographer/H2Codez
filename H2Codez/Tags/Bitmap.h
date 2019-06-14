#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct bitmap_block
{
	__int16 type;
	__int16 format;
	__int16 usage;
	__int16 flags;
	float detailFadeFactor01;
	int sharpenAmount01;
	float bumpHeightRepeats;
	__int16 size;
	__int16 field_16;
	__int16 colorPlateWidthPixels;
	__int16 colorPlateHeightPixels;
	byte_ref compressedColorPlateData;
	byte_ref processedPixelData;
	float blurFilterSize010Pixels;
	float alphaBias11;
	__int16 mipmapCountLevels;
	__int16 spriteUsage;
	__int16 spriteSpacing;
	__int16 forceFormat;
	tag_block_ref sequences;
	tag_block_ref bitmaps;
	char colorCompressionQuality1127;
	char alphaCompressionQuality1127;
	char overlap;
	char colorSubsampling;
};
CHECK_STRUCT_SIZE(bitmap_block, 0x70);

struct __declspec(align(4)) bitmap_data_block
{
	DWORD signature;
	__int16 widthPixels;
	__int16 heightPixels;
	BYTE depthPixels;
	byte moreFlags;
	short type;
	short format;
	__int16 flags;
	__int16 registrationPoint_x;
	__int16 registrationPoint_y;
	__int16 mipmapCount;
	__int16 lowDetailMipmapCount;
	int pixelsOffset;
	int padding11[3];
	int padding12[3];
	float compression_percentage[3];
	int padding14[3];
	datum owner_tag;
	BYTE padding16[4];
	void *bitmap_data;
	BYTE padding18[4];
	BYTE padding19[20];
	BYTE padding20[4];
};
CHECK_STRUCT_SIZE(bitmap_data_block, 0x74);
