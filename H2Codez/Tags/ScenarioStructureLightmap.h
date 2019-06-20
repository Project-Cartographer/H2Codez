#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"
#include "GlobalGeometry.h"

struct lightmap_geometry_section_block
{
	global_geometry_section_info_struct_block geometryInfo;

	global_geometry_block_info_struct_block geometryBlockInfo;

	tag_block<global_geometry_section_struct_block> cacheData;

};

struct structure_lightmap_group_block
{

	enum Type : short
	{
		Normal = 0,
	};
	Type type;

	enum Flags : short
	{
		Unused = 0x1,
	};
	Flags flags;
	int structureChecksum;
	tag_block<> sectionPalette;

	tag_block<> writablePalettes;

	// TagReference("bitm")
	tag_reference bitmapGroup;
	tag_block<lightmap_geometry_section_block> clusters;

	tag_block<> clusterRenderInfo;

	tag_block<lightmap_geometry_section_block> poopDefinitions;

	tag_block<> lightingEnvironments;

	tag_block<> geometryBuckets;

	tag_block<> instanceRenderInfo;

	tag_block<> instanceBucketRefs;

	tag_block<> sceneryObjectInfo;

	tag_block<> sceneryObjectBucketRefs;
};
CHECK_STRUCT_SIZE(structure_lightmap_group_block, 156);

struct scenario_structure_lightmap_block
{
	float searchDistanceLowerBound;
	float searchDistanceUpperBound;
	float luminelsPerWorldUnit;
	float outputWhiteReference;
	float outputBlackReference;
	float outputSchlickParameter;
	float diffuseMapScale;
	float sunScale;
	float skyScale;
	float indirectScale;
	float prtScale;
	float surfaceLightScale;
	float scenarioLightScale;
	float lightprobeInterpolationOveride;
	BYTE padding8[72];
	tag_block<structure_lightmap_group_block> lightmapGroups;

	BYTE padding9[12];
	tag_block<> errors;

	BYTE padding10[104];
};
CHECK_STRUCT_SIZE(scenario_structure_lightmap_block, 268);
