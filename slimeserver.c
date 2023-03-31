#include "slimeserver.h"
#include "network.h"

#include <stdio.h>

void SlimeServer_InitAndStartServer( SlimeServer *server )
{
    // Init the game manager (allocs gamestate)        
    SlimeGame_Init( &server->game );
    server->currentTick = 0;
    server->tickLeftover = 0.0f;


	// Start the network stuff
	server->address.host = ENET_HOST_ANY;
	/* Bind the server to port 1234. */
	server->address.port = 1234;

	server->enetServer = enet_host_create(&(server->address) /* the address to bind the server host to */,
		32      /* allow up to 32 clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);
}

void SlimeServer_Update( SlimeServer *server, f32 dt )
{
	// Poll network
	ENetEvent netEvent;

	b32 polled = FALSE;
	while (!polled)
	{
		if (enet_host_check_events(server->enetServer, &netEvent) <= 0)
		{
			if (enet_host_service(server->enetServer, &netEvent, 100) <= 0)
			{
				break;
			}
			polled = TRUE;

			// Handle event
			switch (netEvent.type)
			{
				case ENET_EVENT_TYPE_NONE: break;

				case ENET_EVENT_TYPE_CONNECT:
					{
						printf("Client connected... (id %d) IP is 0x%08x\n", netEvent.peer->connectID, netEvent.peer->incomingPeerID);

						// I think peer is safe to hold onto here
						server->peers[server->numPeers++] = netEvent.peer;
						break;
					}

				case ENET_EVENT_TYPE_DISCONNECT:
					{
						printf("Client disconnected. (id %d)\n", netEvent.peer->connectID);
						if (server->numPeers > 1) {
							for (int i = 0; i < server->numPeers; i++)
							{
								if (server->peers[i] == netEvent.peer)
								{
									server->peers[i] = server->peers[server->numPeers - 1];
									break;
								}
							}
							server->numPeers--;
						}
						break;
					}

				case ENET_EVENT_TYPE_RECEIVE: 
					{
						//netEvent.packet->data;
						Header* slimePacket = (Header*)(netEvent.packet->data);
						printf("Packet received from %d, packetype %d\n", netEvent.peer->connectID, slimePacket->packetType );

						// if it's a lobby packet, handle player ready/name etc
						// if it's a command packet, assign it to the command queue
						
						break;
					}

				default: ;
			}
		}
	}

	// Update game tick(s)
    server->tickLeftover += dt;
    while (server->tickLeftover > SIMTICK_TIME) {
        server->tickLeftover -= SIMTICK_TIME;
        SlimeGame_Tick( &(server->game) );
    }

}