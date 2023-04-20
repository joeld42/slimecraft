#ifndef CMDLIST_H
#define CMDLIST_H

#include "common.h"
#include "gamestate.h"

// Will probably be much smaller but just for fun
#define MAX_CMDS (100)
enum {
    Command_UNKNOWN,

    Command_PASS,
    Command_MOVE,
};

typedef struct {
    u32 cmdType; // eg. Command_PASS
    HUnit unit; // Which unit to move
    float targetX;
    float targetY;
} CommandMove;

typedef union {
    u32 cmdType; // eg. Command_PASS, Command_Move    
    CommandMove move;
} Command;

typedef struct CommandTurn_s {
    u32 commsTurn;
    Command cmdForPlayer[MAX_PLAYERS];
} CommandTurn;


typedef struct {
    u32 front;
    u32 back;
    CommandTurn cmds[ MAX_CMDS ];
} CmdList;

u32 CmdList_Size( CmdList *cmdList );
void CmdList_PushCommandForPlayer( CmdList *cmdList, u32 commsTurn, u8 player, Command cmd );
void CmdList_Reset(CmdList* cmdList);
CommandTurn CmdList_PopNextTurn( CmdList *cmdList );

// For Debug GUI
CommandTurn* CmdList_PeekCommand(CmdList* cmdList, int orderIndex );

#endif