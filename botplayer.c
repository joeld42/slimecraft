#include "gamestate.h"
#include "botplayer.h"

Command BotPlayer_ThinkCommand(SlimeGame* game, BotPlayerInfo* botstate)
{
	Command resultCmd = { .cmdType = Command_PASS };

	// TODO: do a move command sometimes

	return resultCmd;
}