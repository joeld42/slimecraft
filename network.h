#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

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