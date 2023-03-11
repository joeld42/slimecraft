#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "common.h"

// Info is game which don't change
typedef struct SlimeGameInfo_t {
    u16 mapSizeX;
    u16 mapSizeY;
} SlimeGameInfo;

typedef struct SlimeGameState_t {
    u32 x;
} SlimeGameState;

typedef struct SlimeGameMgr_t {
    SlimeGameInfo *info;
    SlimeGameState *curr;
    SlimeGameState *next;
} SlimeGameMgr;

// udpate gamestate in place
void GameState_Tick( SlimeGameState *state );

// Compute checksum for gamestate
u64 GameState_Checksum( SlimeGameState *state );

void GameMgr_Reset( SlimeGameMgr *gameMgr );

// Test stuff
void DoStuff();

#endif