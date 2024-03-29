#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "common.h"

typedef struct
{
	b32 playerReady;
} PlayerInfo;

// Info is game state which doesn't change
typedef struct  {
    u16 mapSizeX;
    u16 mapSizeY;
    u8 numPlayers;
	PlayerInfo playerInfo[MAX_PLAYERS];
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

	// Tick
	u32 tick;

	// Checksum for sync. Calculated as if this were 0
	u32 checksum;

    // Unit List
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
u32 GameState_Checksum(SlimeGameState* state, u32* checksum);

// == Unit utils
HUnit SlimeGame_SpawnUnit( SlimeGame *game, u8 player, u8 type );

u16 SlimeGame_GetNumUnits( SlimeGame *game );
HUnit SlimeGame_GetUnitByIndex( SlimeGame *game, u16 index );
SimVec2 SlimeGame_GetUnitPosition( SlimeGame *game, HUnit unit );
void SlimeGame_SetUnitPosition( SlimeGame *game, HUnit unit, SimVec2 pos );

SimVec2 SlimeGame_GetUnitAction( SlimeGame *game, HUnit unit, u8 *outAction );

void SlimeGame_Init( SlimeGame *game );
void SlimeGame_Reset( SlimeGame *game, int numPlayers );
void SlimeGame_Tick( SlimeGame *game, CommandTurn *cmds );

u32 SlimeGame_CurrentCommsTick(u32 tick); // What was the comms tick number for the given tick?
u32 SlimeGame_NextCommsTick(u32 tick); // What tick number is the next comms tick
u32 SlimeGame_NextCommandTick(u32 tick); // What tick number should we issue commands for for this tick?

// Test stuff
void DoStuff();

#endif