#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "common.h"

// Info is game which don't change
typedef struct  {
    u16 mapSizeX;
    u16 mapSizeY;
    u8 numPlayers;
} SlimeGameInfo;

typedef struct {
    float x; // TODO make fixed point
    float y;
} SimVec2;

enum {
    UnitType_EMPTY,
    UnitType_DEAD,

    UnitType_FOUNDER,
    UnitType_FIELD_COMMANDER,
    UnitType_WORKER
};

enum {
    Action_IDLE,
    Action_MOVING,
    ACTION_HARVESTING
};

typedef struct  {
    u8 unitType; 
    u8 player;   
    u8 action;
    SimVec2 pos;
    SimVec2 target;
} SlimeGameUnit;

#define MAX_UNITS (200)
#define MAX_BUILDINGS (100)
typedef struct {
    
    u16 numUnits;
    SlimeGameUnit units[MAX_UNITS]; 
    
    RNGState rng; // TODO have more specialized RNGs
} SlimeGameState;

typedef struct {
    SlimeGameInfo *info;
    SlimeGameState *prev;
    SlimeGameState *curr;
} SlimeGame;

// Compute checksum for gamestate
u64 GameState_Checksum( SlimeGameState *state );


// == Unit utils
HUnit SlimeGame_SpawnUnit( SlimeGame *game, u8 player, u8 type );

u16 SlimeGame_GetNumUnits( SlimeGame *game );
HUnit SlimeGame_GetUnitByIndex( SlimeGame *game, u16 index );
SimVec2 SlimeGame_GetUnitPosition( SlimeGame *game, HUnit unit );
void SlimeGame_SetUnitPosition( SlimeGame *game, HUnit unit, SimVec2 pos );

void SlimeGame_Init( SlimeGame *game );
void SlimeGame_Reset( SlimeGame *game );
void SlimeGame_Tick( SlimeGame *game );

// Test stuff
void DoStuff();

#endif