#include <stdio.h>
#include <enet/enet.h>

#include "gamestate.h"

int main( int argc, char *argv[] )
{
    printf("Slimeserv CLI.\n");
    SlimeGameState *gs = (SlimeGameState*)malloc( sizeof(SlimeGameState));
    printf("Gamestate size is %zu\n", sizeof( SlimeGameState));

    // Initialize enet
    if (enet_initialize () != 0)
    {
        fprintf (stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit (enet_deinitialize);

}