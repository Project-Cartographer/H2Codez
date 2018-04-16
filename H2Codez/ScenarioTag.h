#pragma once
#include "BasicTagTypes.h"

struct actor_starting_locations_block
{
	OldStringId name;
	RealPoint3D position;
	short referenceFrame;
	BYTE padding1[2];
	RealAngle2D facingyawPitchDegrees;

	enum Flags : int
	{
		InitiallyAsleep = 0x1,
		InfectionFormExplode = 0x2,
		Na = 0x4,
		AlwaysPlace = 0x8,
		InitiallyHidden = 0x10,
	};
	Flags flags;
	// BlockIndex1("character_palette_block")
	short characterType;
	// BlockIndex1("scenario_weapon_palette_block")
	short initialWeapon;
	// BlockIndex1("scenario_weapon_palette_block")
	short initialSecondaryWeapon;
	BYTE padding2[2];
	// BlockIndex1("scenario_vehicle_palette_block")
	short vehicleType;

	enum SeatType : short
	{
		DEFAULT = 0,
		Passenger = 1,
		Gunner = 2,
		Driver = 3,
		SmallCargo = 4,
		LargeCargo = 5,
		NODriver = 6,
		NOVehicle = 7,
	};
	SeatType seatType;

	enum GrenadeType : short
	{
		None = 0,
		HumanGrenade = 1,
		CovenantPlasma = 2,
	};
	GrenadeType grenadeType;
	/// number of cretures in swarm if a swarm is spawned at this location
	short swarmCount;
	StringId actorVariantName;
	StringId vehicleVariantName;
	/// before doing anything else, the actor will travel the given distance in its forward direction
	float initialMovementDistance;
	// BlockIndex1("scenario_vehicle_block")
	short emitterVehicle;

	enum InitialMovementMode : short
	{
		Default = 0,
		Climbing = 1,
		Flying = 2,
	};
	InitialMovementMode initialMovementMode;
	char placementScript[32];
	BYTE padding3[2];
	BYTE padding4[2];
};
static_assert(sizeof(actor_starting_locations_block) == 100, "invalid 'actor_starting_locations_block' size");

struct squads_block
{
	/* filt */
	char name[32];

	enum Flags : int
	{
		Unused = 0x1,
		NeverSearch = 0x2,
		StartTimerImmediately = 0x4,
		NoTimerDelayForever = 0x8,
		MagicSightAfterTimer = 0x10,
		AutomaticMigration = 0x20,
		DEPRECATED = 0x40,
		RespawnEnabled = 0x80,
		Blind = 0x100,
		Deaf = 0x200,
		Braindead = 0x400,
		_3dFiringPositions = 0x800,
		InitiallyPlaced = 0x1000,
		UnitsNotEnterableByPlayer = 0x2000,
	};
	Flags flags;

	enum Team : short
	{
		Default = 0,
		Player = 1,
		Human = 2,
		Covenant = 3,
		Flood = 4,
		Sentinel = 5,
		Heretic = 6,
		Prophet = 7,
		Unused8 = 8,
		Unused9 = 9,
		Unused10 = 10,
		Unused11 = 11,
		Unused12 = 12,
		Unused13 = 13,
		Unused14 = 14,
		Unused15 = 15,
	};
	Team team;
	// BlockIndex1("squad_groups_block")
	short parent;
	float squadDelayTimeSeconds;
	/// initial number of actors on normal difficulty
	short normalDiffCount;
	/// initial number of actors on insane difficulty (hard difficulty is midway between normal and insane)
	short insaneDiffCount;

	enum MajorUpgrade : short
	{
		Normal = 0,
		Few = 1,
		Many = 2,
		None = 3,
		All = 4,
	};
	MajorUpgrade majorUpgrade;
	BYTE padding2[2];
	//BYTE padding3[12];
	// Explaination("Actor defaults", "The following default values are used for spawned actors")
	// BlockIndex1("scenario_vehicle_palette_block")
	short vehicleType;
	// BlockIndex1("character_palette_block")
	short characterType;
	// BlockIndex1("zone_block")
	short initialZone;
	BYTE padding4[2];
	// BlockIndex1("scenario_weapon_palette_block")
	short initialWeapon;
	// BlockIndex1("scenario_weapon_palette_block")
	short initialSecondaryWeapon;

	enum GrenadeType : short
	{
		GrenadeNONE = 0,
		HumanGrenade = 1,
		CovenantPlasma = 2,
	};
	GrenadeType grenadeType;
	// BlockIndex1("orders_block")
	short initialOrder;
	StringId vehicleVariant;
	//BYTE padding5[8];
	tag_block<actor_starting_locations_block> startingLocations;

	char placementScript[32];
	BYTE padding6[2];
	BYTE padding7[2];
};

static_assert(sizeof(squads_block) == 120, "bad 'squads_block' size");

struct scnr_tag
{
	tag_ref unused_sbsp;
	tag_block_ref skies;
	__int16 type;
	__int16 flags;
	tag_block_ref childScenarios;
	float localNorth;
	tag_block_ref predictedResources;
	tag_block_ref functions;
	byte_ref editorScenarioData;
	tag_block_ref comments;
	tag_block_ref noNameField0;
	tag_block_ref objectNames;
	tag_block_ref scenery;
	tag_block_ref sceneryPalette;
	tag_block_ref bipeds;
	tag_block_ref bipedPalette;
	tag_block_ref vehicles;
	tag_block_ref vehiclePalette;
	tag_block_ref equipment;
	tag_block_ref equipmentPalette;
	tag_block_ref weapons;
	tag_block_ref weaponPalette;
	tag_block_ref deviceGroups;
	tag_block_ref machines;
	tag_block_ref machinePalette;
	tag_block_ref controls;
	tag_block_ref controlPalette;
	tag_block_ref lightFixtures;
	tag_block_ref lightFixturesPalette;
	tag_block_ref soundScenery;
	tag_block_ref soundSceneryPalette;
	tag_block_ref lightVolumes;
	tag_block_ref lightVolumesPalette;
	tag_block_ref playerStartingProfile;
	tag_block_ref playerStartingLocations;
	tag_block_ref killTriggerVolumes;
	tag_block_ref recordedAnimations;
	tag_block_ref netgameFlags;
	tag_block_ref netgameEquipment;
	tag_block_ref startingEquipment;
	tag_block_ref bSPSwitchTriggerVolumes;
	tag_block_ref decals;
	tag_block_ref decalsPalette;
	tag_block_ref detailObjectCollectionPalette;
	tag_block_ref stylePalette;
	tag_block_ref squadGroups;
	tag_block<squads_block> squads;
	tag_block_ref zones;
	tag_block_ref missionScenes;
	tag_block_ref characterPalette;
	tag_block_ref aIPathfindingData;
	tag_block_ref aIAnimationReferences;
	tag_block_ref aIScriptReferences;
	tag_block_ref aIRecordingReferences;
	tag_block_ref aIConversations;
	byte_ref scriptSyntaxData;
	byte_ref scriptStringData;
	tag_block_ref scripts;
	tag_block_ref globals;
	tag_block_ref references;
	tag_block_ref sourceFiles;
	tag_block_ref scriptingData;
	tag_block_ref cutsceneFlags;
	tag_block_ref cutsceneCameraPoints;
	tag_block_ref cutsceneTitles;
	tag_ref customObjectNames;
	tag_ref chapterTitleText;
	tag_ref hUDMessages;
	tag_block_ref structureBSPs;
	tag_block_ref scenarioResources;
	tag_block_ref scenarioResources_unused;
	tag_block_ref hsUnitSeats;
	tag_block_ref scenarioKillTriggers;
	tag_block_ref hsSyntaxDatums;
	tag_block_ref orders;
	tag_block_ref triggers;
	tag_block_ref backgroundSoundPalette;
	tag_block_ref soundEnvironmentPalette;
	tag_block_ref weatherPalette;
	tag_block_ref eMPTYSTRING;
	tag_block_ref eMPTYSTRING1;
	tag_block_ref eMPTYSTRING2;
	tag_block_ref eMPTYSTRING3;
	tag_block_ref eMPTYSTRING4;
	tag_block_ref scenarioClusterData;
	int s_objectsalts[32];
	tag_block_ref spawnData;
	tag_ref soundEffectCollection;
	tag_block_ref crates;
	tag_block_ref cratesPalette;
	tag_ref globalLighting;
	tag_block_ref atmosphericFogPalette;
	tag_block_ref planarFogPalette;
	tag_block_ref flocks;
	tag_ref subtitles;
	tag_block_ref decorators;
	tag_block_ref creatures;
	tag_block_ref creaturesPalette;
	tag_block_ref decoratorsPalette;
	tag_block_ref BSPTransitionVolumes;
	tag_block_ref structureBSPLighting;
	tag_block_ref EditorFolders;
	tag_block_ref levelData;
	tag_ref territoryLocationNames;
	char padding[8];
	tag_block_ref missionDialogue;
	tag_ref objectives;
	tag_block_ref interpolators;
	tag_block_ref sharedReferences;
	tag_block_ref screenEffectReferences;
	tag_block_ref simulationDefinitionTable;
};
