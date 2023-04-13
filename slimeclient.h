#ifndef SLIME_CLIENT_H
#define SLIME_CLIENT_H

#include <stdbool.h>

#include "common.h"
#include "network.h"
#include "gamestate.h"

#include <enet/enet.h>

typedef struct {

	// Manages the gamestate 
	SlimeGame game;


	// Info for stepping the gamestate.
	u32 currentTick; // Count of sim ticks    
	float tickLeftover; // leftover time in seconds

	// Network stuff
	ENetAddress address;
	ENetHost* enetClient;

	ENetPeer* serverPeer;

} SlimeClient;

void SlimeClient_InitAndConnect(SlimeClient* client);
bool SlimeClient_Update(SlimeClient* client);
void SlimeClient_Teardown(SlimeClient* client);

#endif