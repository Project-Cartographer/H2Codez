#pragma once
#pragma pack(1)
#include "Object.h"
#include "Common/BasicTagTypes.h"

struct scenery_block
{
    object_block object;
    // Explaination("Pathfinding", "Indicate whether, by default, we should create pathfinding data for this type of scenery")

    enum PathfindingPolicy : short
    {
        PathfindingCUTOUT = 0,
        PathfindingSTATIC = 1,
        PathfindingDYNAMIC = 2,
        PathfindingNONE = 3,
    };
    PathfindingPolicy pathfindingPolicy;

    enum Flags : short
    {
        PhysicallySimulatesstimulates = 0x1,
    };
    Flags flags;
    // Explaination("Lightmapping", "Indicate whether, by default, how we should lightmap this type of scenery")

    enum LightmappingPolicy : short
    {
        PerVertex = 0,
        PerPixelnotImplemented = 1,
        Dynamic = 2,
    };
    LightmappingPolicy lightmappingPolicy;
    byte padding86[2];
};
CHECK_STRUCT_SIZE(scenery_block, sizeof(object_block) + 8);
