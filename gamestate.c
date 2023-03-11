#include "common.h"
#include "gamestate.h"

#include <stdlib.h>
#include <string.h>

void GameState_Tick( SlimeGameState *state ) {

}

// Compute checksum for gamestate
u64 GameState_Checksum( SlimeGameState *state ) {
    return 0;
}

void GameMgr_Reset( SlimeGameMgr *gameMgr ) {

    // Set default game info
    memset( gameMgr->info, 0, sizeof(SlimeGameInfo) );
    gameMgr->info->mapSizeX = 128;
    gameMgr->info->mapSizeY = 128;

    memset( gameMgr->curr, 0, sizeof(SlimeGameState) );
    memset( gameMgr->next, 0, sizeof(SlimeGameState) );

}
