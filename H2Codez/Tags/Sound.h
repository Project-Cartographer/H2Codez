#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"
#include "GlobalGeometry.h"

struct simple_platform_sound_playback_struct_block
{
	tag_block<> noNameField0;


	enum Flags : int
	{
		Use3dRadioHack = 0x1,
	};
	Flags flags;
	byte padding51[8];
	tag_block<> filter;

	tag_block<> pitchLfo;

	tag_block<> filterLfo;

	tag_block<> soundEffect;

};
CHECK_STRUCT_SIZE(simple_platform_sound_playback_struct_block, 72);

struct sound_platform_sound_playback_block
{
	simple_platform_sound_playback_struct_block playbackDefinition;

	tag_block<g_null_block> noNameField0;

};
CHECK_STRUCT_SIZE(sound_platform_sound_playback_block, 84);

struct sound_playback_parameters_struct_block
{
	/// the distance below which this sound no longer gets louder
	float minimumDistanceWorldUnits;
	/// the distance beyond which this sound is no longer audible
	float maximumDistanceWorldUnits;
	/// fraction of requests to play this sound that will be ignored (0 means always play.)(fraction)
	float skipFraction;
	float maximumBendPerSecondCents;
	// Explaination("randomization", "these settings control random variation of volume and pitch. the second parameter gets clipped to the first.")

	/// sound's random gain will start here
	float gainBaseDB;
	/// sound's gain will be randomly modulated within this range
	float gainVarianceDB;
	/// the sound's pitch will be modulated randomly within this range.
	short_bounds randomPitchBoundsCents;
	// Explaination("directional sounds", "these settings allow sounds to be directional, fading as they turn away from the listener")
	/// within the cone defined by this angle and the sound's direction, the sound plays with a gain of 1.0.
	float innerConeAngleDegrees;
	/// outside the cone defined by this angle and the sound's direction, the sound plays with a gain of OUTER CONE GAIN. (0 means the sound does not attenuate with direction.)
	float outerConeAngleDegrees;
	/// the gain to use when the sound is directed away from the listener
	float outerConeGainDB;

	enum Flags : int
	{
		OverrideAzimuth = 0x1,
		Override3dGain = 0x2,
		OverrideSpeakerGain = 0x4,
	};
	Flags flags;
	float azimuth;
	float positionalGainDB;
	float firstPersonGainDB;
};
CHECK_STRUCT_SIZE(sound_playback_parameters_struct_block, 56);

struct sound_scale_modifiers_struct_block
{
	real_bounds gainModifierDB;
	short_bounds pitchModifierCents;
	real_bounds skipFractionModifier;
};
CHECK_STRUCT_SIZE(sound_scale_modifiers_struct_block, 20);

struct sound_promotion_parameters_struct_block
{
	tag_block<> promotionRules;

	tag_block<> noNameField0;

	byte padding57[12];
};
CHECK_STRUCT_SIZE(sound_promotion_parameters_struct_block, 36);

struct sound_permutation_raw_info_block
{
	/* snpl */
	string_id skipFractionName;
	/****************************************
	* definition_name: sound_samples
	* flags: 1
	* alignment_bit: 0
	* byteswap_proc: 0x0049ec90
	****************************************/
	// DataSize(16777216)
	byte_ref samples;
	/****************************************
	* definition_name: sound_mouth_data
	* flags: 0
	* alignment_bit: 0
	****************************************/
	// DataSize(8192)
	byte_ref mouthData;
	/****************************************
	* definition_name: sound_lipsync_data
	* flags: 0
	* alignment_bit: 0
	****************************************/
	// DataSize(1048576)
	byte_ref lipsyncData;
	tag_block<> markers;


	enum Compression : short
	{
		NonebigEndian = 0,
		XboxAdpcm = 1,
		ImaAdpcm = 2,
		NonelittleEndian = 3,
		Wma = 4,
	};
	Compression compression;

	enum Language : byte
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
	byte padding66[1];
};
CHECK_STRUCT_SIZE(sound_permutation_raw_info_block, 80);

struct sound_definition_language_permutation_info_block
{
	tag_block<sound_permutation_raw_info_block> rawInfoBlock;
};
CHECK_STRUCT_SIZE(sound_definition_language_permutation_info_block, 12);

struct sound_extra_info_block
{
	tag_block<sound_definition_language_permutation_info_block> languagePermutationInfo;

	tag_block<> encodedPermutationSection;

	global_geometry_block_info_struct_block geometryBlockInfo;
};
CHECK_STRUCT_SIZE(sound_extra_info_block, 64);

struct sound_block
{
	enum Flags : int
	{
		FitToAdpcmBlocksize = 0x1,
		SplitLongSoundIntoPermutations = 0x2,
		AlwaysSpatializealwaysPlayAs3dSoundEvenInFirstPerson = 0x4,
		NeverObstructdisableOcclusionobstructionForThisSound = 0x8,
		InternalDontTouch = 0x10,
		UseHugeSoundTransmission = 0x20,
		LinkCountToOwnerUnit = 0x40,
		PitchRangeIsLanguage = 0x80,
		DontUseSoundClassSpeakerFlag = 0x100,
		DontUseLipsyncData = 0x200,
	};
	Flags flags;

	enum Class : char
	{
		ProjectileImpact = 0,
		ProjectileDetonation = 1,
		ProjectileFlyby = 2,
		WeaponFire = 4,
		WeaponReady = 5,
		WeaponReload = 6,
		WeaponEmpty = 7,
		WeaponCharge = 8,
		WeaponOverheat = 9,
		WeaponIdle = 10,
		WeaponMelee = 11,
		WeaponAnimation = 12,
		ObjectImpacts = 13,
		ParticleImpacts = 14,
		UnitFootsteps = 18,
		UnitDialog = 19,
		UnitAnimation = 20,
		VehicleCollision = 22,
		VehicleEngine = 23,
		VehicleAnimation = 24,
		DeviceDoor = 26,
		DeviceMachinery = 28,
		DeviceStationary = 29,
		Music = 32,
		AmbientNature = 33,
		AmbientMachinery = 34,
		HugeAss = 36,
		ObjectLooping = 37,
		CinematicMusic = 38,
		CortanaMission = 45,
		CortanaCinematic = 46,
		MissionDialog = 47,
		CinematicDialog = 48,
		ScriptedCinematicFoley = 49,
		GameEvent = 50,
		Ui = 51,
		Test = 52,
		MultilingualTest = 53,
	};
	Class sound_class;

	enum SampleRate : char
	{
		_22kHz = 0,
		_44kHz = 1,
		_32kHz = 2,
	};
	SampleRate sampleRate;

	enum NoNameField0 : char
	{
		None = 0,
		OutputFrontSpeakers = 1,
		OutputRearSpeakers = 2,
		OutputCenterSpeakers = 3,
	};
	NoNameField0 noNameField0;

	enum ImportType : char
	{
		Unknown = 0,
		Singleshot = 1,
		Singlelayer = 2,
		Multilayer = 3,
	};
	ImportType importType;
	sound_playback_parameters_struct_block playback;

	sound_scale_modifiers_struct_block scale;

	// Explaination("import properties", "")
	char padding49[2];

	enum Encoding : char
	{
		Mono = 0,
		Stereo = 1,
		Codec = 2,
	};
	Encoding encoding;

	enum Compression : char
	{
		NonebigEndian = 0,
		XboxAdpcm = 1,
		ImaAdpcm = 2,
		NonelittleEndian = 3,
		Wma = 4,
	};
	Compression compression;
	sound_promotion_parameters_struct_block promotion;

	char padding50[12];
	/// pitch ranges allow multiple samples to represent the same sound at different pitches
	tag_block<> pitchRanges;

	tag_block<sound_platform_sound_playback_block> platformParameters;

	tag_block<sound_extra_info_block> rawPermutations;

};
