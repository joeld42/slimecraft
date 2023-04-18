#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

#define SLIMECRAFT_PORT (20123)

enum
{
	SC_NETCHANNEL_Lobby,
	SC_NETCHANNEL_Game,

	SC_NETCHANNEL_NUMCHANNELS
};

enum
{
	PacketType_NULL,

	PacketType_COMMAND,
	PacketType_RESETGAME,  // Start or reset game
	PacketType_DEBUG,
};

typedef struct
{
	uint8_t packetType;
} Header;


// A fancier version could sync the current game state here but for
// this we just reset it with the new number of players
typedef struct
{
	Header header; // PacketType_ResetGame
	u8 numPlayers;
	u8 assignedPlayerId; 
} PktResetGame;

typedef struct
{
	Header header; // PacketType_DEBUG
	char msg[64];
} PktDebug;

#endif