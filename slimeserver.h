#ifndef SLIMESERVER_H
#define SLIMESERVER_H

#include "common.h"
#include "gamestate.h"
#include "cmdlist.h"

typedef struct {
    
    // Manages the gamestate 
    SlimeGame game;

    // Gather and rebroadcast the commands
    CmdList cmdList;

    // Info for stepping the gamestate.
    u32 currentTick; // Count of sim ticks    
    float tickLeftover; // leftover time in seconds
} SlimeServer;




#endif