#ifndef SLIMESERVER_H
#define SLIMESERVER_H

#include "common.h"
#include "gamestate.h"
#include "cmdlist.h"

#include <enet/enet.h>

// Might want more than players for spectators or logging
#define MAX_PEERS (12)


typedef struct {
    
    // Manages the gamestate 
    SlimeGame game;

    // Gather and rebroadcast the commands
    CmdList cmdList;

    // Info for stepping the gamestate.
    u32 currentTick; // Count of sim ticks    
    float tickLeftover; // leftover time in seconds

	// Network stuff
	ENetAddress address;
	ENetHost* enetServer;

	// Peers
	u32 numPeers;
	ENetPeer* peers[MAX_PEERS];

} SlimeServer;


void SlimeServer_InitAndStartServer( SlimeServer *server );
void SlimeServer_Update( SlimeServer *server, f32 dt );
void SlimeServer_Teardown(SlimeServer* server);

#endif