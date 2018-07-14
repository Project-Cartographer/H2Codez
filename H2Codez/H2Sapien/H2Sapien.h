#pragma once

struct video_settings {
	enum display_mode : DWORD
	{
		unset = 0,
		windowed,
		fullscreen
	};

	display_mode DisplayMode = display_mode::windowed;
	DWORD AspectRatio = 0;
	struct screen_info {
		DWORD x = 800;
		DWORD y = 600;
		DWORD refresh_rate = 60;
	};
	screen_info ScreenInfo;
	DWORD unk1 = 0;
	DWORD Brightness = 3;
	DWORD Gamma = 2;
	DWORD AntiAliasing = 0;
	DWORD HubArea = 2;
	DWORD SafeArea = 0;

	enum level_of_detail : DWORD 
	{
		high = 0,
		med,
		low
	};
	level_of_detail LevelOfDetail = level_of_detail::high;
};
static_assert(sizeof(video_settings) == 0x30, "invalid 'video_settings' size");

class H2SapienPatches {
public:
	static void Init();
};