#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct model_variant_permutation_block
{
    string_id permutationName;
    byte padding149[1];

    enum Flags : byte
    {
        CopyStatesToAllPermutations = 0x1,
    };
    Flags flags;
    byte padding150[2];
    float probability0inf;
    struct model_variant_state_block
    {
        string_id permutationName;
        byte padding153[1];

        enum PropertyFlags : byte
        {
            Blurred = 0x1,
            HellaBlurred = 0x2,
            Shielded = 0x4,
        };
        PropertyFlags propertyFlags;

        enum State : short
        {
            Default = 0,
            MinorDamage = 1,
            MediumDamage = 2,
            MajorDamage = 3,
            Destroyed = 4,
        };
        State state;
        // TagReference("effe")
        /// played while the model is in this state
        tag_reference loopingEffect;
        string_id loopingEffectMarkerName;
        /// (fraction)
        float initialProbability;
    };
    CHECK_STRUCT_SIZE(model_variant_state_block, 32);
    tag_block<model_variant_state_block> states;

    byte padding151[5];
    byte padding152[7];
};
CHECK_STRUCT_SIZE(model_variant_permutation_block, 36);

struct model_variant_region_block
{
    string_id regionName;
    byte padding136[1];
    byte padding137[1];
    // BlockIndex1("model_variant_block")
    short parentVariant;
    tag_block<model_variant_permutation_block> permutations;

    short sortOrder;
    byte padding138[2];
};

struct model_variant_object_block
{
    string_id parentMarker;
    string_id childMarker;
    // TagReference("obje")
    tag_reference childObject;
};
CHECK_STRUCT_SIZE(model_variant_object_block, 24);

struct model_variant_block
{
    string_id name;
    byte padding117[16];
    tag_block<model_variant_region_block> regions;

    tag_block<model_variant_object_block> objects;

    byte padding118[8];
    string_id dialogueSoundEffect;
    // TagReference("udlg")
    tag_reference dialogue;
};
CHECK_STRUCT_SIZE(model_variant_block, 72);

struct model_block
{
    // Explaination("MODEL", "")
    /* hlmt */
    // TagReference("mode")
    tag_reference renderModel;
    // TagReference("coll")
    tag_reference collisionModel;
    // TagReference("jmad")
    tag_reference animation;
    // TagReference("phys")
    tag_reference physics;
    // TagReference("phmo")
    tag_reference physicsmodel;

    float disappearDistanceWorldUnits;
    float beginFadeDistanceWorldUnits;
    byte padding98[4];
    float reduceToL1WorldUnitssuperLow;
    float reduceToL2WorldUnitslow;
    float reduceToL3WorldUnitsmedium;
    float reduceToL4WorldUnitshigh;
    float reduceToL5WorldUnitssuperHigh;
    byte padding99[4];

    enum ShadowFadeDistance : short
    {
        FadeAtSuperHighDetailLevel = 0,
        FadeAtHighDetailLevel = 1,
        FadeAtMediumDetailLevel = 2,
        FadeAtLowDetailLevel = 3,
        FadeAtSuperLowDetailLevel = 4,
        FadeNever = 5,
    };
    ShadowFadeDistance shadowFadeDistance;
    byte padding100[2];
    tag_block<model_variant_block> variants;

    tag_block<> materials;

    tag_block<> newDamageInfo;

    tag_block<> targets;

    tag_block<> noNameField4;

    tag_block<> noNameField5;

    byte padding101[4];
    tag_block<> modelObjectData;

    // Explaination("more stuff", "")
    // TagReference("udlg")
    /// The default dialogue tag for this model (overriden by variants)
    tag_reference defaultDialogue;
    // TagReference("shad")
    tag_reference uNUSED;

    enum Flags : int
    {
        ActiveCamoAlwaysOn = 0x1,
        ActiveCamoAlwaysMerge = 0x2,
        ActiveCamoNeverMerge = 0x4,
    };
    Flags flags;
    /// The default dialogue tag for this model (overriden by variants)
    string_id defaultDialogueEffect;

    // ArrayCount(32)
    byte renderonlyNodeFlags[32];


    // ArrayCount(32)
    byte renderonlySectionFlags[32];


    enum RuntimeFlags : int
    {
        ContainsRuntimeNodes = 0x1,
    };
    RuntimeFlags runtimeFlags;
    tag_block<> scenarioLoadParameters;

    // Explaination("HOLOGRAM", "hologram shader is applied whenever the control function from it's object is non-zero")
    // TagReference("shad")
    tag_reference hologramShader;
    string_id hologramControlFunction;
};
CHECK_STRUCT_SIZE(model_block, 348);
