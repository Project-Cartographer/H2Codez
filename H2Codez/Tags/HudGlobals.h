#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct global_new_hud_globals_constants_struct_block
{
	// tag_reference("")
	tag_reference primaryMessageSound;
	// tag_reference("")
	tag_reference secondaryMessageSound;
	string_id bootGrieferString;
	string_id cannotBootGrieferString;
	// tag_reference("shad")
	tag_reference trainingShader;
	// tag_reference("bitm")
	tag_reference humanTrainingTopRight;
	// tag_reference("bitm")
	tag_reference humanTrainingTopCenter;
	// tag_reference("bitm")
	tag_reference humanTrainingTopLeft;
	// tag_reference("bitm")
	tag_reference humanTrainingMiddle;
	// tag_reference("bitm")
	tag_reference eliteTrainingTopRight;
	// tag_reference("bitm")
	tag_reference eliteTrainingTopCenter;
	// tag_reference("bitm")
	tag_reference eliteTrainingTopLeft;
	// tag_reference("bitm")
	tag_reference eliteTrainingMiddle;
};
CHECK_STRUCT_SIZE(global_new_hud_globals_constants_struct_block, 184);

struct global_new_hud_globals_struct_block
{
	// tag_reference("unic")
	tag_reference hudText;
	tag_block_ref dashlights;

	tag_block_ref waypointArrows;

	struct hud_waypoint_block
	{
		// tag_reference("bitm")
		tag_reference bitmap;
		// tag_reference("shad")
		tag_reference shader;
		short onscreenSequenceIndex;
		short occludedSequenceIndex;
		short offscreenSequenceIndex;
		BYTE padding153[2];
	};
	CHECK_STRUCT_SIZE(hud_waypoint_block, 40);
	tag_block<hud_waypoint_block> waypoints;

	tag_block_ref hudSounds;

	tag_block_ref playerTrainingData;

	global_new_hud_globals_constants_struct_block constants;

};
CHECK_STRUCT_SIZE(global_new_hud_globals_struct_block, 260);

struct hud_globals_block
{
	// Explaination("Messaging parameters", "")

	enum Anchor : short
	{
		TopLeft = 0,
		TopRight = 1,
		BottomLeft = 2,
		BottomRight = 3,
		Center = 4,
		Crosshair = 5,
	};
	Anchor anchor;
	BYTE padding170[2];
	BYTE padding171[32];
	point2d anchorOffset;
	float widthScale;
	float heightScale;

	enum ScalingFlags : short
	{
		DontScaleOffset = 0x1,
		DontScaleSize = 0x2,
	};
	ScalingFlags scalingFlags;
	BYTE padding172[2];
	BYTE padding173[20];
	// tag_reference("bitm")
	tag_reference obsolete1;
	// tag_reference("bitm")
	tag_reference obsolete2;
	float upTime;
	float fadeTime;
	colour_rgba iconColor;
	colour_rgba textColor;
	float textSpacing;
	// tag_reference("unic")
	tag_reference itemMessageText;
	// tag_reference("bitm")
	tag_reference iconBitmap;
	// tag_reference("unic")
	tag_reference alternateIconText;
	tag_block<hud_button_icon_block> buttonIcons;

	// Explaination("HUD HELP TEXT COLOR", "")
	colour_argb defaultColor;
	colour_argb flashingColor;
	float flashPeriod;
	/// time between flashes
	float flashDelay;
	short numberOfFlashes;

	enum FlashFlags : short
	{
		ReverseDefaultflashingColors = 0x1,
	};
	FlashFlags flashFlags;
	/// time of each flash
	float flashLength;
	colour_argb disabledColor;
	BYTE padding174[4];
	// Explaination("Other hud messaging data", "")
	// tag_reference("hmt ")
	tag_reference hudMessages;
	// Explaination("Objective colors", "")
	colour_argb defaultColor;
	colour_argb flashingColor;
	float flashPeriod;
	/// time between flashes
	float flashDelay;
	short numberOfFlashes;

	enum FlashFlags : short
	{
		ReverseDefaultflashingColors = 0x1,
	};
	FlashFlags flashFlags;
	/// time of each flash
	float flashLength;
	colour_argb disabledColor;
	short uptimeTicks;
	short fadeTicks;
	// Explaination("Waypoint parameters", "The offset values are how much the waypoint rectangle border is offset from the safe camera bounds")
	float topOffset;
	float bottomOffset;
	float leftOffset;
	float rightOffset;
	BYTE padding175[32];
	// tag_reference("bitm")
	tag_reference arrowBitmap;
	tag_block_ref waypointArrows;

	BYTE padding176[80];
	// Explaination("Multiplayer parameters", "")
	float hudScaleInMultiplayer;
	BYTE padding177[256];
	// Explaination("Hud globals", "")
	BYTE padding178[16];
	float motionSensorRange;
	/// how fast something moves to show up on the motion sensor
	float motionSensorVelocitySensitivity;
	float motionSensorScaleDONTTOUCHEVER;
	rect2d defaultChapterTitleBounds;
	BYTE padding179[44];
	// Explaination("Hud damage indicators", "")
	short topOffset;
	short bottomOffset;
	short leftOffset;
	short rightOffset;
	BYTE padding180[32];
	// tag_reference("bitm")
	tag_reference indicatorBitmap;
	short sequenceIndex;
	short multiplayerSequenceIndex;
	colour_argb color;
	BYTE padding181[16];
	// Explaination("Hud timer definitions", "")
	// Explaination("Not much time left flash color", "")
	colour_argb defaultColor;
	colour_argb flashingColor;
	float flashPeriod;
	/// time between flashes
	float flashDelay;
	short numberOfFlashes;

	enum FlashFlags : short
	{
		ReverseDefaultflashingColors = 0x1,
	};
	FlashFlags flashFlags;
	/// time of each flash
	float flashLength;
	colour_argb disabledColor;
	BYTE padding182[4];
	// Explaination("Time out flash color", "")
	colour_argb defaultColor;
	colour_argb flashingColor;
	float flashPeriod;
	/// time between flashes
	float flashDelay;
	short numberOfFlashes;

	enum FlashFlags : short
	{
		ReverseDefaultflashingColors = 0x1,
	};
	FlashFlags flashFlags;
	/// time of each flash
	float flashLength;
	colour_argb disabledColor;
	BYTE padding183[4];
	BYTE padding184[40];
	// tag_reference("bitm")
	tag_reference carnageReportBitmap;
	// Explaination("Hud crap that wouldn't fit anywhere else", "")
	short loadingBeginText;
	short loadingEndText;
	short checkpointBeginText;
	short checkpointEndText;
	// tag_reference("snd!")
	tag_reference checkpointSound;
	BYTE padding185[96];
	global_new_hud_globals_struct_block newGlobals;

};

constexpr size_t size = sizeof(hud_globals_block);
CHECK_STRUCT_SIZE(hud_globals_block, 1364);
