#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct interface_tag_references
{
	// tag_ref("bitm")
	tag_ref obsolete1;
	// tag_ref("bitm")
	tag_ref obsolete2;
	// tag_ref("colo")
	tag_ref screenColorTable;
	// tag_ref("colo")
	tag_ref hudColorTable;
	// tag_ref("colo")
	tag_ref editorColorTable;
	// tag_ref("colo")
	tag_ref dialogColorTable;
	// tag_ref("hudg")
	tag_ref hudGlobals;
	// tag_ref("bitm")
	tag_ref motionSensorSweepBitmap;
	// tag_ref("bitm")
	tag_ref motionSensorSweepBitmapMask;
	// tag_ref("bitm")
	tag_ref multiplayerHudBitmap;
	// tag_ref("")
	tag_ref noNameField0;
	// tag_ref("hud#")
	tag_ref hudDigitsDefinition;
	// tag_ref("bitm")
	tag_ref motionSensorBlipBitmap;
	// tag_ref("bitm")
	tag_ref interfaceGooMap1;
	// tag_ref("bitm")
	tag_ref interfaceGooMap2;
	// tag_ref("bitm")
	tag_ref interfaceGooMap3;
	// tag_ref("wgtz")
	tag_ref mainmenuUiGlobals;
	// tag_ref("wgtz")
	tag_ref singleplayerUiGlobals;
	// tag_ref("wgtz")
	tag_ref multiplayerUiGlobals;
};
CHECK_STRUCT_SIZE(interface_tag_references, 304);

struct ai_globals_block
{
	float dangerBroadlyFacing;
	BYTE padding10[4];
	float dangerShootingNear;
	BYTE padding11[4];
	float dangerShootingAt;
	BYTE padding12[4];
	float dangerExtremelyClose;
	BYTE padding13[4];
	float dangerShieldDamage;
	float dangerExetendedShieldDamage;
	float dangerBodyDamage;
	float dangerExtendedBodyDamage;
	BYTE padding14[48];
	// tag_ref("adlg")
	tag_ref globalDialogueTag;
	string_id defaultMissionDialogueSoundEffect;
	BYTE padding15[20];
	float jumpDownWutick;
	float jumpStepWutick;
	float jumpCrouchWutick;
	float jumpStandWutick;
	float jumpStoreyWutick;
	float jumpTowerWutick;
	float maxJumpDownHeightDownWu;
	float maxJumpDownHeightStepWu;
	float maxJumpDownHeightCrouchWu;
	float maxJumpDownHeightStandWu;
	float maxJumpDownHeightStoreyWu;
	float maxJumpDownHeightTowerWu;
	real_vector2d hoistStepWus;
	real_vector2d hoistCrouchWus;
	real_vector2d hoistStandWus;
	BYTE padding16[24];
	real_vector2d vaultStepWus;
	real_vector2d vaultCrouchWus;
	BYTE padding17[48];
	struct ai_globals_gravemind_block
	{
		float minRetreatTimeSecs;
		float idealRetreatTimeSecs;
		float maxRetreatTimeSecs;
	};
	CHECK_STRUCT_SIZE(ai_globals_gravemind_block, 12);
	tag_block<ai_globals_gravemind_block> gravemindProperties;

	BYTE padding18[48];
	/// A target of this scariness is offically considered scary (by combat dialogue, etc.)
	float scaryTargetThrehold;
	/// A weapon of this scariness is offically considered scary (by combat dialogue, etc.)
	float scaryWeaponThrehold;
	float playerScariness;
	float berserkingActorScariness;
};
CHECK_STRUCT_SIZE(ai_globals_block, 372);

struct globals_block
{
	BYTE padding6[172];

	enum Language : int
	{
		English = 0,
		Japanese = 1,
		German = 2,
		French = 3,
		Spanish = 4,
		Italian = 5,
		Korean = 6,
		Chinese = 7,
		Portuguese = 8,
	};
	Language language;
	tag_block_ref havokCleanupResources;

	tag_block_ref collisionDamage;

	tag_block_ref soundGlobals;

	tag_block<ai_globals_block> aiGlobals;

	tag_block_ref damageTable;

	tag_block<g_null_block> noNameField1;

	tag_block_ref sounds;

	tag_block_ref camera;

	tag_block_ref playerControl;

	tag_block_ref difficulty;

	tag_block_ref grenades;

	tag_block_ref rasterizerData;

	tag_block<interface_tag_references> interfaceTags;

	tag_block_ref weaponListupdateweaponlistEnumInGameglobalsh;

	tag_block_ref cheatPowerups;

	tag_block_ref multiplayerInformation;

	tag_block_ref playerInformation;

	tag_block_ref playerRepresentation;

	tag_block_ref fallingDamage;

	tag_block_ref oldMaterials;

	tag_block_ref materials;

	tag_block_ref multiplayerUI;

	tag_block_ref profileColors;

	// tag_ref("mulg")
	tag_ref multiplayerGlobals;
	tag_block_ref runtimeLevelData;

	tag_block_ref uiLevelData;

	// Explaination("Default global lighting", "")
	// tag_ref("gldf")
	tag_ref defaultGlobalLighting;
	BYTE padding7[252];
};
CHECK_STRUCT_SIZE(globals_block, 760);
