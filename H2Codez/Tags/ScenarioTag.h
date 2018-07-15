#pragma once
#include "../Common/BasicTagTypes.h"

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
	tag_block_ref squads;
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
	tag_block<cs_script_data_block> scriptingData;
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
