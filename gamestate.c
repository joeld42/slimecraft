#include "common.h"
#include "gamestate.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// Compute checksum for gamestate
u64 GameState_Checksum( SlimeGameState *state ) {
    return 0;
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
    // For the moment, indexes and handes are the same but that 
    // will change when units can be destroyed
    return (HUnit)index;
}


SimVec2 SlimeGame_GetUnitPosition( SlimeGame *game, HUnit unit ) {
    SlimeGameState *state = game->curr;
    assert(unit < state->numUnits);
    return state->units[ unit ].pos;
}

void SlimeGame_SetUnitPosition( SlimeGame *game, HUnit unit, SimVec2 pos ) {
    SlimeGameState *state = game->curr;
    assert(unit < state->numUnits);
    state->units[unit].pos = pos;
}

// TODO: this shouldn't exist, orders should go through Commands
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
    
    SlimeGame_Reset( game );
};

void SlimeGame_Reset( SlimeGame *game ) {

    // Set default game info    
    memset( game->info, 0, sizeof(SlimeGameInfo) );
    game->info->mapSizeX = 128;
    game->info->mapSizeY = 128;

    memset( game->curr, 0, sizeof(SlimeGameState) );    

    // Initialize the RNG
    RNG_Init( &(game->curr->rng) );

    // set up player state
    game->info->numPlayers = 20;

    // spawn some Founders
    for (int i = 0; i < game->info->numPlayers; i++ ) {
        HUnit hu = SlimeGame_SpawnUnit( game, i, UnitType_FOUNDER );

        SimVec2 startPos;
        startPos.x = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeX );
        startPos.y = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeY );
        SlimeGame_SetUnitPosition( game, hu, startPos );

        SimVec2 movePos;
        movePos.x = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeX );
        movePos.y = RNG_NextFloatRange( &(game->curr->rng), 0, (f32)game->info->mapSizeY );
        SlimeGame_OrderUnitMove( game, hu, movePos );
    }


    // Copy current state to prev just for consistency
    memcpy( game->prev, game->curr, sizeof(SlimeGameState) );
}

void SlimeGame_Tick( SlimeGame *game ) {
    // Copy current state to previous
    memcpy( game->prev, game->curr, sizeof( SlimeGameState ) );

    // Tick current state
    float dt = 1.0 / 10.0;
    float speed = 1.0f * dt;
    SlimeGameState *state = game->curr;
    for (int i=0; i < state->numUnits; i++ ) {
        SlimeGameUnit *u = state->units + i;
        if (u->action == Action_MOVING) {
            SimVec2 dir = (SimVec2){ u->target.x - u->pos.x,
                                     u->target.y - u->pos.y };
            f32 d = sqrtf( dir.x * dir.x + dir.y *dir.y);
            if (d < speed) {
                u->pos = u->target;
                u->action = Action_IDLE;
            } else {
                dir.x /= d;
                dir.y /= d;
                u->pos.x += dir.x * speed;
                u->pos.y += dir.y * speed;
            }
        }
        
    }

}
