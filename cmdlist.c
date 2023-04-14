#include "cmdlist.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

u32 CmdList_Size( CmdList *cmdList ) {
    if (cmdList->front > cmdList->back) {
        return (cmdList->back + MAX_CMDS) - cmdList->front;
    } else {
        return cmdList->back - cmdList->front;
    }
}

CommandTurn* CmdList_PeekCommand( CmdList *cmdList, int orderIndex)
{
	u32 index = (orderIndex + cmdList->front) % MAX_CMDS;
	return cmdList->cmds + index;
}

void CmdList_PushCommandForPlayer( CmdList *cmdList, u32 commsTurn, u8 player, Command cmd )
{
    // push empty command turns on until we have a slot for
    // the new command    
    while (cmdList->cmds[cmdList->back].commsTurn < commsTurn ) {
        u32 lastTurn = cmdList->cmds[cmdList->back].commsTurn;
        cmdList->back++;
        
        // overflow?
        assert( cmdList->back != cmdList->front);

        if (cmdList->back == MAX_CMDS) {
            // wrap
            cmdList->back = 0;
        }
        CommandTurn *cmdTurn = cmdList->cmds + cmdList->back;        
        cmdTurn->commsTurn = lastTurn + 1;
    }

    // Now find the right slot for our commsTurn
    // Could be more clever about this but there should only be a few entries
    // at any time so this should be fast
    u32 currNdx = cmdList->front;
    while (cmdList->cmds[currNdx].commsTurn < commsTurn) {
        currNdx++;
        if (currNdx == MAX_CMDS) {
            currNdx = 0;
        }
    }
    
    CommandTurn *cmdTurn = cmdList->cmds + currNdx;

    // now set the command
    assert( player < MAX_PLAYERS );
    cmdTurn->cmdForPlayer[ player ] = cmd;

}

CommandTurn CmdList_PopNextTurn( CmdList *cmdList )
{
    u32 retIndex = cmdList->front;

    // Advance and wrap around if needed
    cmdList->front++;
    if (cmdList->front == MAX_CMDS) {
        cmdList->front = 0;
    }
    CommandTurn result = cmdList->cmds[retIndex];

    // clear out the command for future
    memset( cmdList->cmds + retIndex, 0, sizeof( CommandTurn) );

    return result;
}
