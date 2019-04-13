#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct scenario_structure_bsp_reference_block
{
	BYTE padding26[16];
	// TagReference("sbsp")
	tag_ref structureBSP;
	// TagReference("ltmp")
	tag_ref structureLightmap;
	BYTE padding27[4];
	float uNUSEDRadianceEstSearchDistance;
	BYTE padding28[4];
	float uNUSEDLuminelsPerWorldUnit;
	float uNUSEDOutputWhiteReference;
	BYTE padding29[8];

	enum Flags : short
	{
		DefaultSkyEnabled = 0x1,
	};
	Flags flags;
	BYTE padding30[2];
	// BlockIndex1("scenario_sky_reference_block")
	short defaultSky;
	BYTE padding31[2];
};
CHECK_STRUCT_SIZE(scenario_structure_bsp_reference_block, 84);

#pragma region cs_script_data_block


struct cs_point_block
{
	char name[32];
	real_point3d position;
	short referenceFrame;
	BYTE padding21[2];
	int surfaceIndex;
	real_euler_angles2d facingDirection;
};
CHECK_STRUCT_SIZE(cs_point_block, 60);


struct cs_point_set_block
{
	char name[32];
	tag_block<cs_point_block> points;

	// BlockIndex1("scenario_structure_bsp_reference_block")
	short bspIndex;
	short manualReferenceFrame;

	enum Flags : int
	{
		ManualReferenceFrame = 0x1,
		TurretDeployment = 0x2,
	};
	Flags flags;
};
CHECK_STRUCT_SIZE(cs_point_set_block, 52);

struct cs_script_data_block
{
	tag_block<cs_point_set_block> pointSets;

	BYTE padding[120];
};
CHECK_STRUCT_SIZE(cs_script_data_block, 132);

#pragma endregion

#pragma region orders_block

struct zone_set_block
{
	enum AreaType : short
	{
		Deault = 0,
		Search = 1,
		Goal = 2,
	};
	AreaType areaType;
	BYTE padding41[2];
	// BlockIndex1("zone_block")
	short zone;
	// BlockIndex2(GetBlockProc = 0x005c0d70, IsValidSourceBlockProc = 0x005c0dc0))
	short area;
};
CHECK_STRUCT_SIZE(zone_set_block, 8);


struct secondary_zone_set_block
{

	enum AreaType : short
	{
		Deault = 0,
		Search = 1,
		Goal = 2,
	};
	AreaType areaType;
	BYTE padding45[2];
	// BlockIndex1("zone_block")
	short zone;
	// BlockIndex2(GetBlockProc = 0x005c0d70, IsValidSourceBlockProc = 0x005c0dc0))
	short area;
};
CHECK_STRUCT_SIZE(secondary_zone_set_block, 8);


struct secondary_set_trigger_block
{
	struct trigger_references
	{

		enum TriggerFlags : int
		{
			Not = 0x1,
		};
		TriggerFlags triggerFlags;
		// BlockIndex1("triggers_block")
		short trigger;
		BYTE padding50[2];
	};
	CHECK_STRUCT_SIZE(trigger_references, 8);

	enum CombinationRule : short
	{
		OR = 0,
		AND = 1,
	};
	CombinationRule combinationRule;

	enum DialogueType : short
	{
		None = 0,
		Advance = 1,
		Charge = 2,
		FallBack = 3,
		Retreat = 4,
		Moveone = 5,
		Arrival = 6,
		EnterVehicle = 7,
		ExitVehicle = 8,
		FollowPlayer = 9,
		LeavePlayer = 10,
		Support = 11,
	};
	/// when this ending is triggered, launch a dialogue event of the given type
	DialogueType dialogueType;
	tag_block<trigger_references> triggers;
};
CHECK_STRUCT_SIZE(secondary_set_trigger_block, 16);


struct order_ending_block
{
	// BlockIndex1("orders_block")
	short nextOrder;

	enum CombinationRule : short
	{
		OR = 0,
		AND = 1,
	};
	CombinationRule combinationRule;
	float delayTime;

	enum DialogueType : short
	{
		None = 0,
		Advance = 1,
		Charge = 2,
		FallBack = 3,
		Retreat = 4,
		Moveone = 5,
		Arrival = 6,
		EnterVehicle = 7,
		ExitVehicle = 8,
		FollowPlayer = 9,
		LeavePlayer = 10,
		Support = 11,
	};
	/// when this ending is triggered, launch a dialogue event of the given type
	DialogueType dialogueType;
	BYTE padding57[2];
	struct trigger_references
	{

		enum TriggerFlags : int
		{
			Not = 0x1,
		};
		TriggerFlags triggerFlags;
		// BlockIndex1("triggers_block")
		short trigger;
		BYTE padding61[2];
	};
	CHECK_STRUCT_SIZE(trigger_references, 8);

	tag_block<trigger_references> triggers;
};
CHECK_STRUCT_SIZE(order_ending_block, 24);


struct orders_block
{
	/* filt */
	char name[32];
	// BlockIndex1("style_palette_block")
	short style;
	BYTE padding38[2];

	enum Flags : int
	{
		Locked = 0x1,
		AlwaysActive = 0x2,
		DebugOn = 0x4,
		StrictAreaDef = 0x8,
		FollowClosestPlayer = 0x10,
		FollowSquad = 0x20,
		ActiveCamo = 0x40,
		SuppressCombatUntilEngaged = 0x80,
		InhibitVehicleUse = 0x100,
	};
	Flags flags;

	enum ForceCombatStatus : short
	{
		None = 0,
		Asleep = 1,
		Idle = 2,
		Alert = 3,
		Combat = 4,
	};
	ForceCombatStatus forceCombatStatus;
	BYTE padding39[2];
	char entryScript[32];
	WORD scriptIndex;
	// BlockIndex1("squads_block")
	short followSquad;
	float followRadius;
	tag_block<zone_set_block> primaryAreaSet;

	tag_block<secondary_zone_set_block> secondaryAreaSet;

	tag_block<secondary_set_trigger_block> secondarySetTrigger;

	struct special_movement_block
	{

		enum SpecialMovement1 : int
		{
			Jump = 0x1,
			Climb = 0x2,
			Vault = 0x4,
			Mount = 0x8,
			Hoist = 0x10,
			WallJump = 0x20,
			Na = 0x40,
		};
		SpecialMovement1 specialMovement1;
	};
	CHECK_STRUCT_SIZE(special_movement_block, 4);

	tag_block<special_movement_block> specialMovement;

	tag_block<order_ending_block> orderEndings;

};
CHECK_STRUCT_SIZE(orders_block, 144);

#pragma endregion

#pragma region squads_block

struct actor_starting_locations_block
{
	old_string_id name;
	real_point3d position;
	short referenceFrame;
	BYTE padding153[2];
	real_euler_angles2d facingyawPitchDegrees;

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
	BYTE padding154[2];
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
	string_id actorVariantName;
	string_id vehicleVariantName;
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
	WORD scriptIndex;
	BYTE padding156[2];
};


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
		FiringPositions3d = 0x800,
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
	BYTE padding107[2];
	// Explaination("Actor defaults", "The following default values are used for spawned actors")
	// BlockIndex1("scenario_vehicle_palette_block")
	short vehicleType;
	// BlockIndex1("character_palette_block")
	short characterType;
	// BlockIndex1("zone_block")
	short initialZone;
	BYTE padding108[2];
	// BlockIndex1("scenario_weapon_palette_block")
	short initialWeapon;
	// BlockIndex1("scenario_weapon_palette_block")
	short initialSecondaryWeapon;

	enum GrenadeType : short
	{
		none = 0,
		HumanGrenade = 1,
		CovenantPlasma = 2,
	};
	GrenadeType grenadeType;
	// BlockIndex1("orders_block")
	short initialOrder;
	string_id vehicleVariant;
	tag_block<actor_starting_locations_block> startingLocations;

	char placementScript[32];
	WORD scriptIndex;
	BYTE padding110[2];
};
CHECK_STRUCT_SIZE(squads_block, 120);

struct squad_groups_block
{
	char name[32];
	// BlockIndex1("squad_groups_block")
	short parent;
	// BlockIndex1("orders_block")
	short initialOrders;
};
CHECK_STRUCT_SIZE(squad_groups_block, 36);

#pragma endregion


#pragma region hs_scripts_block
struct hs_scripts_block
{
	char name[32];

	enum ScriptType : short
	{
		Startup = 0,
		Dormant = 1,
		Continuous = 2,
		Static = 3,
		Stub = 4,
		CommandScript = 5,
	};
	ScriptType scriptType;

	enum ReturnType : short
	{
		Unparsed = 0,
		SpecialForm = 1,
		FunctionName = 2,
		Passthrough = 3,
		Void = 4,
		Boolean = 5,
		Real = 6,
		Short = 7,
		Long = 8,
		String = 9,
		Script = 10,
		StringId = 11,
		UnitSeatMapping = 12,
		TriggerVolume = 13,
		CutsceneFlag = 14,
		CutsceneCameraPoint = 15,
		CutsceneTitle = 16,
		CutsceneRecording = 17,
		DeviceGroup = 18,
		Ai = 19,
		AiCommandList = 20,
		AiCommandScript = 21,
		AiBehavior = 22,
		AiOrders = 23,
		StartingProfile = 24,
		Conversation = 25,
		StructureBsp = 26,
		Navpoint = 27,
		PointReference = 28,
		Style = 29,
		HudMessage = 30,
		ObjectList = 31,
		Sound = 32,
		Effect = 33,
		Damage = 34,
		LoopingSound = 35,
		AnimationGraph = 36,
		DamageEffect = 37,
		ObjectDefinition = 38,
		Bitmap = 39,
		Shader = 40,
		RenderModel = 41,
		StructureDefinition = 42,
		LightmapDefinition = 43,
		GameDifficulty = 44,
		Team = 45,
		ActorType = 46,
		HudCorner = 47,
		ModelState = 48,
		NetworkEvent = 49,
		Object = 50,
		Unit = 51,
		Vehicle = 52,
		Weapon = 53,
		Device = 54,
		Scenery = 55,
		ObjectName = 56,
		UnitName = 57,
		VehicleName = 58,
		WeaponName = 59,
		DeviceName = 60,
		SceneryName = 61,
	};
	ReturnType returnType;
	int rootExpressionIndex;
};
#pragma endregion

struct scnr_tag
{
	tag_ref unused_sbsp;
	tag_block_ref skies;
	enum Type : short
	{
		Singleplayer = 0,
		Multiplayer = 1,
		Mainmenu = 2,
		MultiplayerShared = 3,
		SingleplayerShared = 4,
	};
	Type type;
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
	tag_block<squad_groups_block> squadGroups;
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
	tag_block<hs_scripts_block> scripts;
	tag_block_ref globals;
	tag_block_ref references;
	tag_block_ref sourceFiles;
	tag_block<cs_script_data_block> scriptingData;
	tag_block_ref cutsceneFlags;
	tag_block_ref cutsceneCameraPoints;
	tag_block_ref cutsceneTitles;
	tag_ref customObjectNames;
	tag_ref chapterTitleText;
	tag_ref hUDMessages;
	tag_block<scenario_structure_bsp_reference_block> structureBSPs;
	tag_block_ref scenarioResources;
	tag_block_ref scenarioResources_unused;
	tag_block_ref hsUnitSeats;
	tag_block_ref scenarioKillTriggers;
	tag_block_ref hsSyntaxDatums;
	tag_block<orders_block> orders;
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
CHECK_STRUCT_SIZE(scnr_tag, 1476);
