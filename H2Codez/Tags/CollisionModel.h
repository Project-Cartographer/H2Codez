#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"
#include "GlobalGeometry.h"

struct collision_model_block
{
    tag_block<global_tag_import_info_block> importInfo;

    tag_block<> errors;


    enum Flags : int
    {
        ContainsOpenEdges = 0x1,
    };
    Flags flags;
    tag_block<> materials;

    tag_block<> regions;

    tag_block<> pathfindingSpheres;

    tag_block<> nodes;
};
CHECK_STRUCT_SIZE(collision_model_block, 76);