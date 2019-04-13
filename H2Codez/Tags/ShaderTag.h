#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct shader_animation_property_block
{

	enum Type : short
	{
		BitmapScaleUniform = 0,
		BitmapScaleX = 1,
		BitmapScaleY = 2,
		BitmapScaleZ = 3,
		BitmapTranslationX = 4,
		BitmapTranslationY = 5,
		BitmapTranslationZ = 6,
		BitmapRotationAngle = 7,
		BitmapRotationAxisX = 8,
		BitmapRotationAxisY = 9,
		BitmapRotationAxisZ = 10,
		Value = 11,
		Color = 12,
		BitmapIndex = 13,
	};
	Type type;
	BYTE padding141[2];
	string_id inputName;
	string_id rangeName;
	float timePeriodSec;
	// Explaination("FUNCTION", "EMPTY STRING")
	/* fned */
	struct mapping_function_block
	{
		/* hide */
		struct byte_block
		{
			BYTE value;
		};
		CHECK_STRUCT_SIZE(byte_block, 1);
		tag_block<byte_block> data;

		/* edih */
	};
	CHECK_STRUCT_SIZE(mapping_function_block, 12);
	mapping_function_block function;
};
CHECK_STRUCT_SIZE(shader_animation_property_block, 28);

struct global_shader_parameter_block
{
	string_id name;

	enum Type : short
	{
		Bitmap = 0,
		Value = 1,
		Color = 2,
		Switch = 3,
	};
	Type type;
	BYTE padding131[2];
	// TagReference("bitm")
	tag_ref bitmap;
	float constValue;
	colour_rgb constColor;
	tag_block<shader_animation_property_block> animationProperties;
};
CHECK_STRUCT_SIZE(global_shader_parameter_block, 52);


struct shader_block
{
	// TagReference("stem")
	tag_ref _template;
	string_id materialName;
	struct shader_properties_block
	{
		// TagReference("bitm")
		tag_ref diffuseMap;
		// TagReference("bitm")
		tag_ref lightmapEmissiveMap;
		colour_rgb lightmapEmissiveColor;
		float lightmapEmissivePower;
		float lightmapResolutionScale;
		float lightmapHalfLife;
		float lightmapDiffuseScale;
		// TagReference("bitm")
		tag_ref alphaTestMap;
		// TagReference("bitm")
		tag_ref translucentMap;
		colour_rgb lightmapTransparentColor;
		float lightmapTransparentAlpha;
		float lightmapFoliageScale;
	};
	CHECK_STRUCT_SIZE(shader_properties_block, 112);
	tag_block<shader_properties_block> runtimeProperties;

	BYTE padding128[2];

	enum Flags : short
	{
		Water = 0x1,
		SortFirst = 0x2,
		NoActiveCamo = 0x4,
	};
	Flags flags;
	tag_block<global_shader_parameter_block> parameters;

	tag_block_ref postprocessDefinition;

	BYTE padding129[4];
	struct predicted_resource_block
	{

		enum Type : short
		{
			Bitmap = 0,
			Sound = 1,
			RenderModelGeometry = 2,
			ClusterGeometry = 3,
			ClusterInstancedGeometry = 4,
			LightmapGeometryObjectBuckets = 5,
			LightmapGeometryInstanceBuckets = 6,
			LightmapClusterBitmaps = 7,
			LightmapInstanceBitmaps = 8,
		};
		Type type;
		short resourceIndex;
		int tagIndex;
	};
	CHECK_STRUCT_SIZE(predicted_resource_block, 8);
	tag_block<predicted_resource_block> predictedResources;

	// TagReference("slit")
	tag_ref lightResponse;

	enum ShaderLODBias : short
	{
		_None = 0,
		_4xSize = 1,
		_2xSize = 2,
		_12Size = 3,
		_14Size = 4,
		_Never = 5,
		_Cinematic = 6,
	};
	ShaderLODBias shaderLODBias;

	enum SpecularType : short
	{
		None = 0,
		Default = 1,
		Dull = 2,
		Shiny = 3,
	};
	SpecularType specularType;

	enum LightmapType : short
	{
		Diffuse = 0,
		DefaultSpecular = 1,
		DullSpecular = 2,
		ShinySpecular = 3,
	};
	LightmapType lightmapType;
	BYTE padding130[2];
	float lightmapSpecularBrightness;
	float lightmapAmbientBias11;
	struct long_block
	{
		int bitmapGroupIndex;
	};
	CHECK_STRUCT_SIZE(long_block, 4);
	tag_block<long_block> postprocessProperties;

	float addedDepthBiasOffset;
	float addedDepthBiasSlopeScale;
};
CHECK_STRUCT_SIZE(shader_block, 128);
