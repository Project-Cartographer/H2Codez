#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct global_geometry_part_block_new
{

	enum Type : short
	{
		NotDrawn = 0,
		OpaqueShadowOnly = 1,
		OpaqueShadowCasting = 2,
		OpaqueNonshadowing = 3,
		Transparent = 4,
		LightmapOnly = 5,
	};
	Type type;

	enum Flags : short
	{
		Decalable = 0x1,
		NewPartTypes = 0x2,
		DislikesPhotons = 0x4,
		OverrideTriangleList = 0x8,
		IgnoredByLightmapper = 0x10,
	};
	Flags flags;
	// BlockIndex1("global_geometry_material_block")
	short material;
	short stripStartIndex;
	short stripLength;
	short firstSubpartIndex;
	short subpartCount;
	byte maxNodesVertex;
	byte contributingCompoundNodeCount;
	// Explaination("CENTROID", "EMPTY STRING")
	real_point3d position;

	byte nodeIndices[4];
	float nodeWeights[3];

	float lodMipmapMagicNumber;
	byte padding6[24];
};
CHECK_STRUCT_SIZE(global_geometry_part_block_new, 72);

struct global_geometry_section_struct_block
{
	tag_block<global_geometry_part_block_new> parts;

	tag_block<> subparts;

	tag_block<> visibilityBounds;

	tag_block<> rawVertices;

	tag_block<> stripIndices;

	/****************************************
	* definition_name: global_section_mopp_code_data
	* flags: 0
	* alignment_bit: 16
	* byteswap_proc: 0x00531b20
	****************************************/
	// DataSize(393216)
	byte_ref visibilityMoppCode;
	tag_block<> moppReorderTable;

	tag_block<> vertexBuffers;

	BYTE padding33[4];
};
CHECK_STRUCT_SIZE(global_geometry_section_struct_block, 108);

struct global_geometry_block_resource_block
{

	enum Type : BYTE
	{
		TagBlock = 0,
		TagData = 1,
		VertexBuffer = 2,
	};
	Type type;
	BYTE padding52[3];
	short primaryLocator;
	short secondaryLocator;
	int resourceDataSize;
	int resourceDataOffset;
};
CHECK_STRUCT_SIZE(global_geometry_block_resource_block, 16);

struct global_geometry_block_info_struct_block
{
	// Explanation("BLOCK INFO", "EMPTY STRING")
	int blockOffset;
	int blockSize;
	int sectionDataSize;
	int resourceDataSize;
	tag_block<global_geometry_block_resource_block> resources;

	BYTE padding34[4];
	short ownerTagSectionOffset;
	BYTE padding35[2];
	BYTE padding36[4];
};
CHECK_STRUCT_SIZE(global_geometry_block_info_struct_block, 40);

struct global_geometry_compression_info_block
{
	real_vector2d positionBoundsX;
	real_vector2d positionBoundsY;
	real_vector2d positionBoundsZ;
	real_vector2d texcoordBoundsX;
	real_vector2d texcoordBoundsY;
	real_vector2d secondaryTexcoordBoundsX;
	real_vector2d secondaryTexcoordBoundsY;
};
CHECK_STRUCT_SIZE(global_geometry_compression_info_block, 56);

enum GeometryClassification : short
{
	Worldspace = 0,
	Rigid = 1,
	RigidBoned = 2,
	Skinned = 3,
	UnsupportedReimport = 4,
};

struct global_geometry_section_info_struct_block
{
	// Explanation("SECTION INFO", "EMPTY STRING")
	short totalVertexCount;
	short totalTriangleCount;
	short totalPartCount;
	short shadowCastingTriangleCount;
	short shadowCastingPartCount;
	short opaquePointCount;
	short opaqueVertexCount;
	short opaquePartCount;
	BYTE opaqueMaxNodesVertex;
	BYTE transparentMaxNodesVertex;
	short shadowCastingRigidTriangleCount;

	GeometryClassification geometryClassification;

	enum GeometryCompressionFlags : short
	{
		CompressedPosition = 0x1,
		CompressedTexcoord = 0x2,
		CompressedSecondaryTexcoord = 0x4,
	};
	GeometryCompressionFlags geometryCompressionFlags;
	tag_block<global_geometry_compression_info_block> bounds;

	BYTE hardwareNodeCount;
	BYTE nodeMapSize;
	short softwarePlaneCount;
	short totalSubpartcont;

	enum SectionLightingFlags : short
	{
		HasLmTexcoords = 0x1,
		HasLmIncRad = 0x2,
		HasLmColors = 0x4,
		HasLmPrt = 0x8,
	};
	SectionLightingFlags sectionLightingFlags;
};
CHECK_STRUCT_SIZE(global_geometry_section_info_struct_block, 44);

struct global_geometry_raw_point_block
{
	real_point3d position;

	int nodeIndicesOLD[4];
	float nodeWeights[4];
	int nodeIndicesNEW[4];

	int useNewNodeIndices;
	int adjustedCompoundNodeIndex;
};
CHECK_STRUCT_SIZE(global_geometry_raw_point_block, 68);

struct global_geometry_rigid_point_group_block
{
	byte rigidNodeIndex;
	byte nodesPoint;
	short pointCount;
};
CHECK_STRUCT_SIZE(global_geometry_rigid_point_group_block, 4);

struct global_geometry_point_data_index_block
{
	short index;
};
CHECK_STRUCT_SIZE(global_geometry_point_data_index_block, 2);

struct global_geometry_point_data_struct_block
{
	tag_block<global_geometry_raw_point_block> rawPoints;

	/****************************************
	* definition_name: global_geometry_runtime_point_data_definition
	* flags: 0
	* alignment_bit: 0
	****************************************/
	// DataSize(1048544)
	byte_ref runtimePointData;
	tag_block<global_geometry_rigid_point_group_block> rigidPointGroups;

	tag_block<global_geometry_point_data_index_block> vertexPointIndices;

};
CHECK_STRUCT_SIZE(global_geometry_point_data_struct_block, 56);

struct global_geometry_material_property_block
{

	enum Type : short
	{
		LightmapResolution = 0,
		LightmapPower = 1,
		LightmapHalfLife = 2,
		LightmapDiffuseScale = 3,
	};
	Type type;
	short intValue;
	float realValue;
};
CHECK_STRUCT_SIZE(global_geometry_material_property_block, 8);


struct global_geometry_material_block
{
	// TagReference("shad")
	tag_reference oldShader;
	// TagReference("shad")
	tag_reference shader;
	tag_block<global_geometry_material_property_block> properties;

	byte padding81[4];
	byte breakableSurfaceIndex;
	byte padding82[3];
};
CHECK_STRUCT_SIZE(global_geometry_material_block, 52);
