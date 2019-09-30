#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct render_model_node_block
{
	/// (old string id)
	string_id name;
	// BlockIndex1("render_model_node_block")
	short parentNode;
	// BlockIndex1("render_model_node_block")
	short firstChildNode;
	// BlockIndex1("render_model_node_block")
	short nextSiblingNode;
	short importNodeIndex;
	real_point3d defaultTranslation;
	real_quaternion defaultRotation;
	real_matrix4x3 inverseMatrix;
	float distanceFromParent;
};
CHECK_STRUCT_SIZE(render_model_node_block, 96);

struct render_model_block
{
	/// old string id
	string_id name;

	enum Flags : short
	{
		RenderModelForceThirdPersonBit = 0x1,
		ForceCarmackreverse = 0x2,
		ForceNodeMaps = 0x4,
		GeometryPostprocessed = 0x8,
	};
	Flags flags;
	byte padding4[2];
	byte padding5[4];
	tag_block<> importInfo;

	tag_block<> compressionInfo;

	tag_block<> regions;

	tag_block<> sections;

	tag_block<> invalidSectionPairBits;

	tag_block<> sectionGroups;

	byte l1SectionGroupIndexsuperLow;
	byte l2SectionGroupIndexlow;
	byte l3SectionGroupIndexmedium;
	byte l4SectionGroupIndexhigh;
	byte l5SectionGroupIndexsuperHigh;
	byte l6SectionGroupIndexhollywood;
	byte padding6[2];
	int nodeListChecksum;
	tag_block<render_model_node_block> nodes;

	tag_block<> nodeMapOLD;

	tag_block<> markerGroups;

	tag_block<> materials;

	tag_block<> errors;

	/// dont draw fp model when camera > this angle cosine (-1,1) Sugg. -0.2. 0 disables.
	float dontDrawOverCameraCosineAngle;
	tag_block<> pRTInfo;

	tag_block<> sectionRenderLeaves;
};
CHECK_STRUCT_SIZE(render_model_block, 184);
