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
	//enet_address_set_host_ip( &(server->address.host))
	//enet_address_set_host(&(server->address), "localhost");
	//enet_address_set_host_ip(&(server->address), "1.0.0.127");
	server->address.port = SLIMECRAFT_PORT;

	server->enetServer = enet_host_create(&(server->address) /* the address to bind the server host to */,
		32      /* allow up to 32 clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);
}


/*
 *OLD EVENT LOOP, i don't really understand why it's structued this way, copypasted
// TODO use a much longer timeout if we're waiting for connections
b32 polled = FALSE;
while (!polled)
{
	if (enet_host_check_events(server->enetServer, &netEvent) <= 0)
	{
		if (enet_host_service(server->enetServer, &netEvent, 100) <= 0)
		{
			printf("ENET: no events.\n");
			break;
		}
		polled = TRUE;

		// Handle event
		switch (netEvent.type)
		{
		}
		}
*/


void SlimeServer_Update( SlimeServer *server, f32 dt )
{
	// Poll network
	ENetEvent netEvent;


	// TODO use a much longer timeout if we're waiting for connections
	int hasEvents = enet_host_service(server->enetServer, &netEvent, 100);
	if (hasEvents < 0)
	{
		printf("ERROR in enet_host_service, error code %d\n", hasEvents);
	}

	if ( hasEvents > 0)
	{
		// Handle event
		switch (netEvent.type)
		{
		case ENET_EVENT_TYPE_NONE:
			printf("event is EVENT_TYPE_NONE\n");
			break;

		case ENET_EVENT_TYPE_CONNECT:
		{
			printf("Client connected... (id %d) IP is 0x%08x\n", netEvent.peer->connectID, netEvent.peer->incomingPeerID);

			// I think peer is safe to hold onto here
			PeerInfo* pi = server->peers + server->numPeers++;
			pi->enetPeer = netEvent.peer;

			pi->playerId = server->game.info->numPlayers;

			// Reset the game state
			SlimeGame_Reset(&(server->game), server->game.info->numPlayers + 1);

				// Send all the peer a reset packet
				static PktResetGame resetPacket;
				resetPacket.header.packetType = PacketType_RESETGAME;
				resetPacket.numPlayers = server->game.info->numPlayers;
				for (int i=0; i < server->numPeers; i++)
				{
					PeerInfo* peer = server->peers + i;
					resetPacket.assignedPlayerId = peer->playerId;
					ENetPacket* packet = enet_packet_create(&resetPacket, sizeof(resetPacket), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send( peer->enetPeer, SC_NETCHANNEL_Lobby, packet);
				}

			break;
		}

		case ENET_EVENT_TYPE_DISCONNECT:
		{
			printf("Client disconnected. (id %d)\n", netEvent.peer->connectID);
			if (server->numPeers > 1) {
				for (u32 i = 0; i < server->numPeers; i++)
				{
					if (server->peers[i].enetPeer == netEvent.peer)
					{
						server->peers[i] = server->peers[server->numPeers - 1];
						break;
					}
				}
			}
			server->numPeers--;

				// fixme, this doesn't work with bots or players, only
			// supported for testing joining/quitting one player
			printf("Resetting, numPeers %d (should be 0)\n", server->numPeers);
			SlimeGame_Reset(&(server->game), 0 );

			break;
		}

		case ENET_EVENT_TYPE_RECEIVE:
		{
			//netEvent.packet->data;
			Header* slimePacket = (Header*)(netEvent.packet->data);
			printf("Packet received from %d, packetype %d\n", netEvent.peer->connectID, slimePacket->packetType);

			if (slimePacket->packetType == PacketType_DEBUG)
			{
				PktDebug* packetMsg = (PktDebug*)slimePacket;
				printf("Debug Message: %s\n", packetMsg->msg);
			}
			else
			{
				printf("Unknown packet type %d\n", slimePacket->packetType);
			}
			// if it's a lobby packet, handle player ready/name etc
			// if it's a command packet, assign it to the command queue

			break;
		}

		default:;
		}

	}


	// Update game tick(s)
    server->tickLeftover += dt;
    while (server->tickLeftover > SIMTICK_TIME) {
        server->tickLeftover -= SIMTICK_TIME;
        SlimeGame_Tick( &(server->game) );
		printf("Tick: %d (netEvents %d), Checksum 0x%08X\n", 
			server->game.curr->tick, hasEvents, server->game.curr->checksum );
    }

}

void SlimeServer_Teardown(SlimeServer* server)
{
	for (int i=0; i < server->numPeers; i++)
	{
		enet_peer_disconnect( server->peers[i].enetPeer, 0 );
	}

	// Wait up to 3 seconds for the disconnect to succeed
	int peersLeft = server->numPeers;
	ENetEvent event;
	while (enet_host_service( server->enetServer, &event, 3000) > 0)
	{
		switch (event.type)
		{
			// drop any packets
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			peersLeft -= 1;
			printf("Disconnect from peer %d succeeded (%d remain)\n", event.peer->connectID, peersLeft);
			if (peersLeft == 0)
			{
				// Everyone disconnected cleanly
				server->numPeers = 0;
				return;
			}
			break;

		default:
			break;
		}
	}

	// If we reach here, some clients didn't respond, so just reset them all
	for (int i=0; i < server->numPeers; i++)
	{
		enet_peer_reset(server->peers[i].enetPeer);
	}
	server->numPeers = 0;
}