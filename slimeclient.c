#include <stdio.h>
#include <string.h>

#include "slimeclient.h"

#include "cmdlist.h"


void SlimeClient_InitAndConnect(SlimeClient* client)
{

	client->address.port = SLIMECRAFT_PORT;
	enet_address_set_host(&(client->address), "localhost");

	client->enetClient = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		SC_NETCHANNEL_NUMCHANNELS,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */);

	if (!client->enetClient)
	{
		printf("ERROR: Failed to create client host.\n");
		return;
	}

	// Now connect to server
	client->serverPeer = enet_host_connect(
		client->enetClient,
		&(client->address),
		SC_NETCHANNEL_NUMCHANNELS,
		(enet_uint32)NULL /* connection data */);
	if (client->serverPeer == NULL)
	{
		printf("ERROR: failed to connect to server.\n");
		return;

	} 

	/* Wait up to 5 seconds for the connection attempt to succeed. */
	ENetEvent event;
	if (enet_host_service(client->enetClient, &event, 5000) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT)
	{
		printf("Connected to %u.%u.%u.%u\n",
			(client->address.host & 0xff000000) >> 24, 
			(client->address.host & 0x00ff0000) >> 16,
			(client->address.host & 0x0000ff00) >> 8,
			(client->address.host & 0x000000ff));


		static PktDebug testPacket;
		testPacket.header.packetType = PacketType_DEBUG;
		strcpy(testPacket.msg, "Hello from client.");

		ENetPacket* packet = enet_packet_create(&testPacket, sizeof(testPacket), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(client->serverPeer, SC_NETCHANNEL_Lobby, packet);
		enet_host_flush(client->enetClient);
	}
	else
	{
		/* Either the 5 seconds are up or a disconnect event was */
		/* received. Reset the peer in the event the 5 seconds   */
		/* had run out without any significant event.            */
		enet_peer_reset(client->serverPeer);

		printf("Connection to server failed.\n");

	}


	// Init the game state
	SlimeGame_Init(&(client->game) );
}

void SlimeClient_ResetGame(SlimeClient* client, int numPlayers)
{
	SlimeGame_Reset(&(client->game), numPlayers);
	CmdList_Reset(&(client->cmdList));
}


bool SlimeClient_Update(SlimeClient* client, f32 dt)
{
	ENetEvent event;
	if (enet_host_service(client->enetClient, &event, 1000) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_NONE:
			{
				printf("event is EVENT_TYPE_NONE\n");
				break;
			}

			case ENET_EVENT_TYPE_CONNECT:
			{
				printf("Connect event... (id %d) IP is 0x%08x\n", 
					event.peer->connectID, event.peer->incomingPeerID);

				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:
			{
				printf("Disconnect event. (id %d)\n", event.peer->connectID);
				return false;
				break;
			}

			case ENET_EVENT_TYPE_RECEIVE:
			{
				Header* slimePacket = (Header*)(event.packet->data);
				printf("Packet received from server %d, zzpacketype %d\n", event.peer->connectID, slimePacket->packetType);

					if (slimePacket->packetType == PacketType_RESETGAME)
					{
						// Yoink! reset game.
						PktResetGame* pktResetGame = (PktResetGame*)slimePacket;
						printf("Got reset packet! my assigned player ID is %d (total players %d)\n",
							pktResetGame->assignedPlayerId,
							pktResetGame->numPlayers);


						// set our client ID and reset the game state
						client->playerID = pktResetGame->assignedPlayerId;
						SlimeClient_ResetGame(client, pktResetGame->numPlayers);
					}

				break;
			}
		}
	}


	// If this is a comms tick, we should have commands for it.
	u32 currTick = client->game.curr->tick;
	u32 commsTick = SlimeGame_CurrentCommsTick(currTick);
	CommandTurn* turnCmds = NULL;
	if ( currTick == commsTick)
	{
		
		turnCmds = CmdList_PeekCommand( &(client->cmdList), 0);
		if ((!turnCmds) || (turnCmds->commsTurn != commsTick))
		{
			printf("TIMEOUT: No commands for comms tick %d, pausing...\n", commsTick );
			client->netpause = true;
		} else
		{
			printf("TICK %d is a comms tick.\n", currTick);
		}
	}

	// Update game tick(s)
	if (!client->netpause) {
		client->tickLeftover += dt;
		while (client->tickLeftover > SIMTICK_TIME) {
			client->tickLeftover -= SIMTICK_TIME;
			SlimeGame_Tick(&(client->game), turnCmds );
			printf("Tick: %d Checksum 0x%08X\n",
				client->game.curr->tick, client->game.curr->checksum);
		}
		//printf("Timeleft %3.2f\n", client->tickLeftover);
	}


	return true;
}

void SlimeClient_Teardown( SlimeClient* client )
{
	enet_host_destroy(client->enetClient);
}