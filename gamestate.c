#include "common.h"
#include "gamestate.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "cmdlist.h"


// Compute checksum for gamestate, from http://home.thep.lu.se/~bjorn/crc/
// This is probably overkill for just checking desync but i'd rather something
// robust for now at least. Note passing in the checksum here is a little weird
// because we eventuall store it in gamestate, and we need to be careful not to
// use that as we're calculating, but keeping it this way incase we need to do
// checksums over multiple blocks in the state
u32 GameState_Checksum( SlimeGameState *state, u32 *checksum ) {

	static u32* crc_table = NULL;

	// Alloc and init crc table if this is the first time
	if (!crc_table)
	{
		printf("Setting up CRC table...\n");
		crc_table = (u32*)malloc(sizeof(u32) * 0x100);
		for (int i=0; i < 0x100; i++)
		{
			u32 r = i;
			for (int j=0; j < 8; j++)
			{
				r = (r & 1 ? 0 : (u32)0xEDB88320L) ^ r >> 1;
			}
			crc_table[i] = r ^ (u32)0xFF000000L; // fly you foools
		}
	}

	u32 crc = *checksum;
	for (size_t i=0; i < sizeof(SlimeGameState); i++)
	{
		crc = crc_table[(u8)* checksum ^ ((u8*)state)[i]] ^ crc >> 8;
	}

	*checksum = crc;
	return crc;
}


// What was the comms tick number for the given tick?
u32 SlimeGame_CurrentCommsTick(u32 tick)
{
	return tick - (tick + SIMTICKS_PER_COMM_TURN) % SIMTICKS_PER_COMM_TURN;
}

// What tick number should we issue commands for for this tick?
u32 SlimeGame_NextCommandTick(u32 tick)
{
	return tick - (tick + SIMTICKS_PER_COMM_TURN) % SIMTICKS_PER_COMM_TURN + (SIMTICKS_PER_COMM_TURN * 2);
}

HUnit SlimeGame_SpawnUnit( SlimeGame *game, u8 player, u8 type ) {
    SlimeGameState *state = game->curr;
    assert( state->numUnits < (MAX_UNITS-1));
    HUnit hu = state->numUnits++;
    SlimeGameUnit *unit = state->units + hu;
    unit->player = player;
    unit->unitType = type;
    unit->pos = (SimVec2){ 0, 0 };
    unit->target = (SimVec2){ 0, 0 };
    unit->action = Action_IDLE;
    return hu;
}


u16 SlimeGame_GetNumUnits( SlimeGame *game ) {
    return game->curr->numUnits;
}

// Note: this won't work anymore once units can be 
// killed or removed, replace this with a proper slotlist
HUnit SlimeGame_GetUnitByIndex( SlimeGame *game, u16 index ) {
    // For the moment, indexes and handles are the same but that 
    // will change when units can be destroyed
    return (HUnit)index;
}


SimVec2 SlimeGame_GetUnitPosition( SlimeGame *game, HUnit unit ) {
    SlimeGameState *state = game->curr;
    assert(unit < state->numUnits);
    return state->units[ unit ].pos;
}

SimVec2 SlimeGame_GetUnitAction( SlimeGame *game, HUnit unit, u8 *outAction )
{
    SlimeGameState *state = game->curr;
    SlimeGameUnit *uu = state->units +unit;
    if (outAction!=NULL) {
        *outAction = uu->action;
    }
    SimVec2 result = (uu->action == Action_MOVING) ? uu->target : uu->pos;
    //SimVec2 result = (SimVec2) { uu->target.x, uu->target.y };
    return result;
}

void SlimeGame_SetUnitPosition( SlimeGame *game, HUnit unit, SimVec2 pos ) {
    SlimeGameState *state = game->curr;
    assert(unit < state->numUnits);
    state->units[unit].pos = pos;
}

// private, orders should go through Commands
void SlimeGame_OrderUnitMove( SlimeGame *game, HUnit unit, SimVec2 targPos ) {
    SlimeGameState *state = game->curr;
    assert(unit < state->numUnits);
    state->units[unit].action = Action_MOVING;
    state->units[unit].target = targPos;    
}

void SlimeGame_Init( SlimeGame *game ) {
    game->info = (SlimeGameInfo*)calloc( 1,  sizeof (SlimeGameInfo) );
    game->curr = (SlimeGameState*)calloc( 1, sizeof(SlimeGameState) );
    game->prev = (SlimeGameState*)calloc( 1, sizeof(SlimeGameState) );
    
    SlimeGame_Reset( game, 0 );
};

void SlimeGame_Reset( SlimeGame *game, int numPlayers ) {

    // Set default game info    
    memset( game->info, 0, sizeof(SlimeGameInfo) );
    game->info->mapSizeX = 128;
    game->info->mapSizeY = 128;

    memset( game->curr, 0, sizeof(SlimeGameState) );    

    // Initialize the RNG
    RNG_Init( &(game->curr->rng) );

    // set up player state
    game->info->numPlayers = numPlayers;

    // spawn some Founders
    for (int i = 0; i < game->info->numPlayers; i++ ) {
        HUnit hu = SlimeGame_SpawnUnit( game, i, UnitType_FOUNDER );

        SimVec2 startPos;
        startPos.x = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeX );
        startPos.y = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeY );
        SlimeGame_SetUnitPosition( game, hu, startPos );

        //SimVec2 movePos;
        //movePos.x = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeX );
        //movePos.y = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeY );
        //SlimeGame_OrderUnitMove( game, hu, movePos );
    }


    // Copy current state to prev just for consistency
    memcpy( game->prev, game->curr, sizeof(SlimeGameState) );
}

void SlimeGame_ApplyCommands(SlimeGame* game, CommandTurn *cmds) {

	// TODO: actually apply the commands
}

void SlimeGame_Tick( SlimeGame *game, CommandTurn *cmds ) {

	if (game->info->numPlayers == 0) {
		printf("Skipping Tick: No players yet.\n");
		return;
	}

	// Copy current state to previous
    memcpy( game->prev, game->curr, sizeof( SlimeGameState ) );
	game->curr->tick = game->prev->tick + 1;

	// Apply commands
	if (cmds != NULL)
	{
		SlimeGame_ApplyCommands(game, cmds);
	}

	// Tick current state
	float dt = 1.0f / 10.0f;
	float speed = 1.0f * dt;
	SlimeGameState* state = game->curr;
	for (int i = 0; i < state->numUnits; i++) {
		SlimeGameUnit* u = state->units + i;
		if (u->action == Action_MOVING) {
			SimVec2 dir = (SimVec2) {
				u->target.x - u->pos.x,
					u->target.y - u->pos.y
			};
			f32 d = sqrtf(dir.x * dir.x + dir.y * dir.y);
			if (d < speed) {
				u->pos = u->target;
				u->action = Action_IDLE;
			}
			else {
				dir.x /= d;
				dir.y /= d;
				u->pos.x += dir.x * speed;
				u->pos.y += dir.y * speed;
			}
		}
	}


	u32 checksum = 0;
	state->checksum = 0L; // Calculate checksum as if it's 0
	state->checksum = GameState_Checksum(state, &checksum);
}
