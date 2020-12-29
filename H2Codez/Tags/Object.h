#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct object_block
{
    byte padding88[2];

    enum Flags : short
    {
        DoesNotCastShadow = 0x1,
        SearchCardinalDirectionLightmapsOnFailure = 0x2,
        Unused = 0x4,
        NotAPathfindingObstacle = 0x8,
        ExtensionOfParentobjectPassesAllFunctionValuesToParentAndUsesParentsMarkers = 0x10,
        DoesNotCauseCollisionDamage = 0x20,
        EarlyMover = 0x40,
        EarlyMoverLocalizedPhysics = 0x80,
        UseStaticMassiveLightmapSamplecastATonOfRaysOnceAndStoreTheResultsForLighting = 0x100,
        ObjectScalesAttachments = 0x200,
        InheritsPlayersAppearance = 0x400,
        DeadBipedsCantLocalize = 0x800,
        AttachToClustersByDynamicSphereuseThisForTheMacGunOnSpacestation = 0x1000,
        EffectsCreatedByThisObjectDoNotSpawnObjectsInMultiplayer = 0x2000,
    };
    Flags flags;
    float boundingRadiusWorldUnits;
    real_point3d boundingOffset;
    /// marine 1.0, grunt 1.4, elite 0.9, hunter 0.5, etc.
    float accelerationScale0inf;

    enum LightmapShadowMode : short
    {
        Default = 0,
        Never = 1,
        Always = 2,
    };
    LightmapShadowMode lightmapShadowMode;

    enum SweetenerSize : byte
    {
        Small = 0,
        Medium = 1,
        Large = 2,
    };
    SweetenerSize sweetenerSize;
    byte padding90[1];
    byte padding91[4];
    /// sphere to use for dynamic lights and shadows. only used if not 0
    float dynamicLightSphereRadius;
    /// only used if radius not 0
    real_point3d dynamicLightSphereOffset;
    string_id defaultModelVariant;
    // TagReference("hlmt")
    tag_reference model;
    // TagReference("bloc")
    tag_reference crateObject;
    // TagReference("shad")
    tag_reference modifierShader;
    // TagReference("effe")
    tag_reference creationEffect;
    // TagReference("foot")
    tag_reference materialEffects;
    tag_block<> aiProperties;
    tag_block<> functions;

    // Explaination("Applying collision damage", "for things that want to cause more or less collision damage")
    /// 0 means 1.  1 is standard scale.  Some things may want to apply more damage
    float applyCollisionDamageScale;
    // Explaination("Game collision damage parameters", "0 - means take default value from globals.globals")
    /// 0-oo
    float minGameAccdefault;
    /// 0-oo
    float maxGameAccdefault;
    /// 0-1
    float minGameScaledefault;
    /// 0-1
    float maxGameScaledefault;
    // Explaination("Absolute collision damage parameters", "0 - means take default value from globals.globals")
    /// 0-oo
    float minAbsAccdefault;
    /// 0-oo
    float maxAbsAccdefault;
    /// 0-1
    float minAbsScaledefault;
    /// 0-1
    float maxAbsScaledefault;
    short hudTextMessageIndex;
    byte padding97[2];
    tag_block<> attachments;

    tag_block<> widgets;

    tag_block<> oldFunctions;

    tag_block<> changeColors;

    tag_block<> predictedResources;
};
CHECK_STRUCT_SIZE(object_block, 256);
