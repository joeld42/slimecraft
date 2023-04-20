#include <stdio.h>
#include <enet/enet.h>

#include "gamestate.h"
#include "slimeclient.h"


int main(int argc, char* argv[])
{
	printf("Slimeserv CLI.\n");
	SlimeGameState* gs = (SlimeGameState*)malloc(sizeof(SlimeGameState));
	printf("Gamestate size is %zu\n", sizeof(SlimeGameState));

	// Test print tick info
#if 1
	for (int i=0; i < 20; i++)
	{
		u32 lastChanceTick = SlimeGame_NextCommsTick(i) - 1;
		bool lastChance = (i == lastChanceTick);
		printf("TICK %d CommTurn %d NextComm %d lastChance (%d) %s\n", i,
			SlimeGame_CurrentCommsTick(i),
			SlimeGame_NextCommandTick(i),
			lastChanceTick,
			lastChance?"YES":"no");
	}
#endif

	// Initialize enet
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize);

	SlimeClient* client = (SlimeClient*)calloc(1, sizeof(SlimeClient));


	// Initialize and connect to server
	SlimeClient_InitAndConnect(client);

	// Update the client until disconnected. Since this is the test client we don't need to
	// tick faster than sim tick
	bool running = true;
	while(running)
	{
		running = SlimeClient_Update(client, SIMTICK_TIME);
		printf("Update...\n" );
	}

	SlimeClient_Teardown(client);
	printf("SlimeTestClient done....\n");


}
