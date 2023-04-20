#ifndef SLIME_CLIENT_H
#define SLIME_CLIENT_H

#include <stdbool.h>

#include "common.h"
#include "network.h"
#include "gamestate.h"
#include "cmdlist.h"

#include <enet/enet.h>

typedef struct {

	// Manages the gamestate 
	SlimeGame game;
	CmdList cmdList;

	// Info for stepping the gamestate.
	//u32 currentTick; // Count of sim ticks    
	float tickLeftover; // leftover time in seconds
	bool netpause; // paused waiting for network commands

	// For the moment each client only owns one player
	int playerID;

	// Network stuff
	ENetAddress address;
	ENetHost* enetClient;

	ENetPeer* serverPeer;

} SlimeClient;

void SlimeClient_InitAndConnect(SlimeClient* client);
bool SlimeClient_Update(SlimeClient* client, f32 dt );
void SlimeClient_Teardown(SlimeClient* client);

void SlimeClient_SendCommand(SlimeClient* client, Command cmd);

#endif