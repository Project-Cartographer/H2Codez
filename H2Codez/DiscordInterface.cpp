#include "stdafx.h"
#include "DiscordInterface.h"


char tool_in_use[128] = "How did you do that??!";

static void updateDiscordPresence()
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = tool_in_use;
	discordPresence.details = "";
	discordPresence.largeImageKey = "canary-large";
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

void DiscordInterface::Init()
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
}

void DiscordInterface::setAppType(H2EK type)
{
	switch (type)
	{
	case H2Tool:
		strncpy(tool_in_use, "Running H2tool.", sizeof(tool_in_use));
		break;
	case H2Sapien:
		strncpy(tool_in_use, "Editing a map using H2Sapien", sizeof(tool_in_use));
		break;
	case H2Guerilla:
		strncpy(tool_in_use, "Editing tags using H2Guerilla", sizeof(tool_in_use));
		break;
	case Invalid:
		strncpy(tool_in_use, "Putting H2Codez in an invalid state for fun and profit", sizeof(tool_in_use));
		break;
	default:
		break;
	}
}
