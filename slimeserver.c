#include "slimeserver.h"

void SlimeServer_InitAndStartServer( SlimeServer *server )
{
    // Init the game manager (allocs gamestate)        
    SlimeGame_Init( &server->game );
    server->currentTick = 0;
    server->tickLeftover = 0.0f;
}

void SlimeServer_Update( SlimeServer *server, f32 dt )
{
    server->tickLeftover += dt;
    while (server->tickLeftover > SIMTICK_TIME) {
        server->tickLeftover -= SIMTICK_TIME;
        SlimeGame_Tick( &(server->game) );
    }

}