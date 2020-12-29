#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"
#include "GlobalGeometry.h"

struct physics_model_block
{

    enum Flags : int
    {
        Unused = 0x1,
    };
    Flags flags;
    float mass;
    /// 0 is default (1). LESS than 1 deactivates less aggressively. GREATER than 1 is more agressive.
    float lowFreqDeactivationScale;
    /// 0 is default (1). LESS than 1 deactivates less aggressively. GREATER than 1 is more agressive.
    float highFreqDeactivationScale;
    byte padding12[24];
    tag_block<> phantomTypes;

    tag_block<> nodeEdges;

    tag_block<> rigidBodies;

    tag_block<> materials;

    tag_block<> spheres;

    tag_block<> multiSpheres;

    tag_block<> pills;

    tag_block<> boxes;

    tag_block<> triangles;

    tag_block<> polyhedra;

    // Explaination("ALL THESE WORLDS ARE YOURS, EXCEPT EUROPA...", "Attempt no landings there.  And you can't edit anything below this point, so why even look at it?")
    tag_block<> polyhedronFourVectors;

    tag_block<> polyhedronPlaneEquations;

    tag_block<> massDistributions;

    tag_block<> lists;

    tag_block<> listShapes;

    tag_block<> mopps;

    /****************************************
    * definition_name: mopp_codes_data
    * flags: 0
    * alignment_bit: 16
    * byteswap_proc: 0x00531b20
    ****************************************/
    // DataSize(1048576)
    byte_ref moppCodes;
    tag_block<> hingeConstraints;

    tag_block<> ragdollConstraints;

    tag_block<> regions;

    tag_block<> nodes;

    tag_block<global_tag_import_info_block> importInfo;

    tag_block<> errors;

    tag_block<> pointToPathCurves;

    tag_block<> limitedHingeConstraints;

    tag_block<> ballAndSocketConstraints;

    tag_block<> stiffSpringConstraints;

    tag_block<> prismaticConstraints;

    tag_block<> phantoms;

};
CHECK_STRUCT_SIZE(physics_model_block, 396);