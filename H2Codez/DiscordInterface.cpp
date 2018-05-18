#include "stdafx.h"
#include "DiscordInterface.h"
#include <process.h>


std::string tool_in_use = "How did you do that??!";
std::string icon_key;
std::string screenshot_key;
std::string image_hover;
bool interface_online;

static void updateDiscordPresence()
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = tool_in_use.c_str();
	discordPresence.details = "";
	discordPresence.largeImageKey = screenshot_key.c_str();
	discordPresence.smallImageKey = icon_key.c_str();
	discordPresence.largeImageText = image_hover.c_str();
	discordPresence.smallImageText = image_hover.c_str();
	Discord_UpdatePresence(&discordPresence);
}

static void handleDiscordReady()
{
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
}

static void handleDiscordError(int errcode, const char* message)
{
}

static void handleDiscordJoin(const char* secret)
{
}

static void handleDiscordSpectate(const char* secret)
{
}

static void handleDiscordJoinRequest(const DiscordJoinRequest* request)
{
}

void DiscordInterface::init()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = handleDiscordReady;
	handlers.disconnected = handleDiscordDisconnected;
	handlers.errored = handleDiscordError;
	handlers.joinGame = handleDiscordJoin;
	handlers.spectateGame = handleDiscordSpectate;
	handlers.joinRequest = handleDiscordJoinRequest;
	Discord_Initialize("379406777500041228", &handlers, 1, NULL);
	updateDiscordPresence();
	interface_online = true;
}

void DiscordInterface::shutdown()
{
	if (interface_online) {
		Discord_Shutdown();
		interface_online = false;
	}
}

void DiscordInterface::setAppType(H2EK type)
{
	switch (type)
	{
	case H2Tool:
		tool_in_use = "Running H2tool.";
		image_hover = "H2Tool";
		screenshot_key = "h2toolscreenshot";
		icon_key = "h2toolicon";
		break;
	case H2Sapien:
		tool_in_use = "Editing scenerio.";
		image_hover = "H2Sapien";
		screenshot_key = "h2sapienscreenshot";
		icon_key = "h2sapienicon";
		break;
	case H2Guerilla:
		tool_in_use = "Editing tags.";
		image_hover = "H2Guerilla";
		screenshot_key = "h2guerillascreenshot";
		icon_key = "h2guerillaicon";
		break;
	case Invalid:
		tool_in_use = "Putting H2Codez in an invalid state for fun and profit";
		break;
	default:
		break;
	}
}
