#include <stdio.h>
#include <enet/enet.h>

#include "gamestate.h"
#include "slimeclient.h"


int main(int argc, char* argv[])
{
	printf("Slimeserv CLI.\n");
	SlimeGameState* gs = (SlimeGameState*)malloc(sizeof(SlimeGameState));
	printf("Gamestate size is %zu\n", sizeof(SlimeGameState));

	// Initialize enet
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize);

	SlimeClient* client = (SlimeClient*)malloc(sizeof(SlimeClient));

	// Initialize and connect to server
	SlimeClient_InitAndConnect(client);

	// Update the client until disconnected
	while(SlimeClient_Update(client))
	{
		printf("Update....\n");
	}

	SlimeClient_Teardown(client);
	printf("SlimeTestClient done....\n");


}
