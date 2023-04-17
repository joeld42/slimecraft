#ifndef BOTPLAYER_H
#define BOTPLAYER_H

// Simple bot player for testing.
// Not smart just does commands

// note: Bot player state doesn't need to get rolled back so
// it doesn't need to be self contained or deterministic
typedef struct 
{
	int playerId; // which player we're controlling

	// TODO more stuff
} BotPlayerInfo;



#endif