#include "cmdlist.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// clear the command list and push enough "Pass" commands to reach the first comms turn
void CmdList_Reset(CmdList* cmdList)
{
	memset(cmdList, 0, sizeof(CmdList));
	cmdList->back++;
	CommandTurn* startCmd = cmdList->cmds + cmdList->front;

	startCmd->commsTurn = 0;
	Command passCmd = { .cmdType = Command_PASS };
	
	for (int i = 0; i < MAX_PLAYERS; i++) {
		startCmd->cmdForPlayer[i] = passCmd;
	}

	printf("After CmdList_Reset, size is %d\n", CmdList_Size(cmdList));
}
u32 CmdList_Size( CmdList *cmdList ) {
    if (cmdList->front > cmdList->back) {
        return (cmdList->back + MAX_CMDS) - cmdList->front;
    } else {
        return cmdList->back - cmdList->front;
    }
}

CommandTurn* CmdList_PeekCommand( CmdList *cmdList, int orderIndex)
{
	if (cmdList->front == cmdList->back)
	{
		// list is empty
		return NULL;
	}

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
		// TODO: zero out command?
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
