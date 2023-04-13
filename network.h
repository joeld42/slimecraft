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
	PacketType_DEBUG,
};

typedef struct
{
	uint8_t packetType;
} Header;

typedef struct
{
	Header header;
	char msg[64];
} PktMessage;

#endif