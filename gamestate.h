#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "common.h"

typedef struct SlimeGameState_t {
    u32 x;
} SlimeGameState;


// udpate gamestate in place
void GameState_Tick( SlimeGameState *state );

// Compute checksum for gamestate
u64 GameState_Checksum( SlimeGameState *state );

// Test stuff
void DoStuff();

#endif