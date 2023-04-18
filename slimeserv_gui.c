
// For math.h on windows
#define _USE_MATH_DEFINES


// Sokol includes
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"

#include "gamestate.h"

#include <math.h>
#include <stdlib.h>

#include <enet/enet.h>

// Game includes
#include <assert.h>

#include "gamestate.h"
#include "slimeserver.h"
#include "slimeclient.h"
#include "botplayer.h"

ImColor playerCol[4] = {
    { 0.94f, 0.02f, 0.51f, 1.0f },
    { 0.02f, 0.84f, 0.95f, 1.0f },
    { 0.65f, 0.96f, 0.01f, 1.0f },
    { 0.95f, 0.79f, 0.01f, 1.0f }
};

#define MAX_BOTS (MAX_PLAYERS)

static struct {
    sg_pass_action pass_action;
    sgp_vec2 points_buffer[4096];
    
    float map_size;
    sgp_vec2 cam_center;
    float cam_zoom;
    
    // Camera bounds
    sgp_vec2 cam_rect_min;
    sgp_vec2 cam_rect_max;
    HUnit hFocusUnit;

    // Stuff for game control
    float tickLeftover;

	// Bot players
	int numBotPlayers;
	BotPlayerInfo bot[MAX_BOTS];
} state;

static struct
{
	bool showPlayerWindow;
	bool showCmdListWindow;

	// Extra info for GUI
	char* playerName[MAX_PLAYERS];
} guiState;

SlimeServer server;


static void init(void) {

	// FIXME: make a log window in IMGUI instead
#ifdef WIN32
#  ifndef NDEBUG
	// Create a win32 console for printfing
	AllocConsole();

	freopen("CONIN$", "rb", stdin);   // reopen stdin handle as console window input
	freopen("CONOUT$", "wb", stdout);  // reopen stout handle as console window output
	freopen("CONOUT$", "wb", stderr); // reopen stderr handle as console
									// window output
#  endif
#endif

	printf("Hello from SlimeServ GUI.\n");

	// Initialize enet
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		exit(EXIT_FAILURE);
	}
	atexit(enet_deinitialize);

    sg_setup(&(sg_desc){
        .context = sapp_sgcontext(),
        .logger.func = slog_func,
    });
    simgui_setup(&(simgui_desc_t){ 0 });

    // Initialize Sokol GP, adjust the size of command buffers for your own use.
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);
    if(!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", 
                    sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }

    // initial clear color
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .action = SG_ACTION_CLEAR, .value = { 0.0f, 0.5f, 1.0f, 1.0 } }
    };

    // Initialize map (todo get this from gamestate)
    state.map_size = 128.0;
    state.cam_center = (sgp_vec2){ state.map_size / 2.0f, state.map_size / 2.0f };    
    state.cam_zoom = state.map_size;

	guiState.showCmdListWindow = true;

    SlimeServer_InitAndStartServer( &server );

}

static void draw_unit( int player, float x, float y ) {

    unsigned int count = 0;
    float step = (2.0f*M_PI)/6.0f;
    float r = 0.3f;
    sgp_vec2 u = {x, y};
    sgp_vec2 vlast;
    for(float theta = 0.0f; theta < 2.0f*M_PI + step*0.5f; theta+=step) {
        sgp_vec2 v = {x + r*cosf(theta), y - r*sinf(theta)};
        if (theta > 0.0) {
            state.points_buffer[count++] = vlast;        
            state.points_buffer[count++] = v;
            state.points_buffer[count++] = u;
        }
        vlast = v;
    }    

    ImColor col = playerCol[ player % 4 ];
    sgp_set_color(col.Value.x, col.Value.y, col.Value.z, 1.0 );
    sgp_draw_filled_triangles( (sgp_triangle*)state.points_buffer, count/3) ;

    

}

static void draw_gamestate() {
    // Get current window size.
    int width = sapp_width(), height = sapp_height();
    float ratio = width/(float)height;

    // Begin recording draw commands for a frame buffer of size (width, height).
    sgp_begin(width, height);
    // Set frame buffer drawing region to (0,0,width,height).
    sgp_viewport(0, 0, width, height);
    
        
    // Set drawing coordinate space to (left=-ratio, right=ratio, top=1, bottom=-1).
    float hsz = state.cam_zoom * 0.5;
    state.cam_rect_min = (sgp_vec2) { (-ratio * hsz) + state.cam_center.x, -hsz + state.cam_center.y };
    state.cam_rect_max = (sgp_vec2) {  (ratio * hsz) + state.cam_center.x,  hsz + state.cam_center.y};
    
    // left, right, top, bottom    
    sgp_project( state.cam_rect_min.x, state.cam_rect_max.x, 
                 state.cam_rect_max.y, state.cam_rect_min.y );

    // Clear the frame buffer.
    sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
    sgp_clear();

    // Draw an animated rectangle that rotates and changes its colors.
    /*
    float time = sapp_frame_count() * sapp_frame_duration();
    float r = sinf(time)*0.5+0.5, g = cosf(time)*0.5+0.5;
    sgp_set_color(r, g, 0.3f, 1.0f);
    sgp_rotate_at(time, 0.0f, 0.0f);
    sgp_draw_filled_rect(-0.5f, -0.5f, 1.0f, 1.0f);
    */

   // Draw map base (todo make this a texture but
   // for now there's no map data)
   sgp_set_color(0.24f, 0.32f, 0.1f, 1.0f );
   sgp_draw_filled_rect(0.0f, 0.0f, state.map_size, state.map_size );

   // Draw map grid
   float gridAlpha = state.cam_zoom / state.map_size;
   if (gridAlpha < 0.0f) gridAlpha = 0.0f;
   if (gridAlpha > 1.0f) gridAlpha = 1.0f; 
   gridAlpha = powf( 1.0f - gridAlpha, 3.0f );
      
   sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
   sgp_set_color(1.0f, 1.0f, 1.0f, gridAlpha );
   u32 npoints;
   sgp_vec2 *p = state.points_buffer;
   float sz = state.map_size;
   for (int i=0; i < (int)state.map_size+1; i++) {
        // horiz grid line
        *p++ = (sgp_vec2){ 0.0f, (float)i };
        *p++ = (sgp_vec2){ sz, (float)i} ;    

        // vert grid line
        *p++ = (sgp_vec2){ (float)i, 0.0f };
        *p++ = (sgp_vec2){ (float)i, sz } ;    
   }
   sgp_draw_lines( (sgp_line*)state.points_buffer, (state.map_size+1) * 2 );

    // Draw units
    SlimeGame *game = &server.game;
    int numUnits = SlimeGame_GetNumUnits( game );
    for (int i=0; i < numUnits; i++ ) {
        HUnit hu = SlimeGame_GetUnitByIndex( game, i );
        SimVec2 pos = SlimeGame_GetUnitPosition( game, hu );
        draw_unit( i % 6, pos.x, pos.y );

        //sgp_set_color(col.Value.x, col.Value.y, col.Value.z, 1.0 );
        //sgp_draw_filled_triangles( (sgp_triangle*)state.points_buffer, count/3) ;
        u8 action;
        SimVec2 targ = SlimeGame_GetUnitAction( game, hu, &action );
        if (action == Action_MOVING) {
            sgp_set_color(0.5f, 1.0f, 0.5f, 1.0 );
            sgp_draw_line( pos.x, pos.y, targ.x, targ.y );
        }
    }
}

static void ShowMainMenubar()
{
	if (igBeginMainMenuBar())
	{
		if (igBeginMenu("Edit", true))
		{
			if (igMenuItem_Bool("Undo", "CTRL+Z", false, true)) {}
			if (igMenuItem_Bool("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			igSeparator();
			if (igMenuItem_Bool("Cut", "CTRL+X", false, true)) {}
			if (igMenuItem_Bool("Copy", "CTRL+C", false, true)) {}
			if (igMenuItem_Bool("Paste", "CTRL+V", false, true)) {}
			igEndMenu();
		}

		if (igBeginMenu("Window", true))
		{
			igCheckbox("Players", &(guiState.showPlayerWindow));
			igCheckbox("CmdList", &(guiState.showCmdListWindow));
			igEndMenu();
		}

		igEndMainMenuBar();
	}
}

static bool BotsFull()
{
	if ((state.numBotPlayers >= MAX_BOTS) ||
		(server.game.info->numPlayers >= MAX_PLAYERS))
	{
		// no space for bots
		return true;
	}
	return false;
}

static BotPlayerInfo *AddBotPlayer()
{
	assert(!BotsFull());
	
	BotPlayerInfo* bot = state.bot + state.numBotPlayers;

	// Initialize bot 
	bot->playerId = server.game.info->numPlayers;

	SlimeGame_Reset(&(server.game), server.game.info->numPlayers+1 );

	return bot;
}

static void ShowPlayerWindow(SlimeGame* game)
{
	igSetNextWindowPos((ImVec2) { 420, 20 }, ImGuiCond_Once, (ImVec2) { 0, 0 });
	igSetNextWindowSize((ImVec2) { 400, 200 }, ImGuiCond_Once);

	if (igBegin("Players", &(guiState.showPlayerWindow), ImGuiWindowFlags_None)) {

		igLabelText("num_players", "%d Active Players", game->info->numPlayers);
		igSeparator();

		if (!BotsFull())
		{
			if (igSmallButton("Add Player"))
			{
				AddBotPlayer();
			}
		}

		igEnd();
	}
}

static void ShowCommandListWindow(SlimeServer* server)
{
	SlimeGame* game = &(server->game);

	igSetNextWindowPos((ImVec2) { 20, 230}, ImGuiCond_Once, (ImVec2) { 0, 0 });
	igSetNextWindowSize((ImVec2) { 400, 200 }, ImGuiCond_Once);


	if (igBegin("CommandList", &(guiState.showCmdListWindow), ImGuiWindowFlags_None)) {
		
		if ( igBeginTable("table-cmds", game->info->numPlayers + 1, ImGuiTableFlags_BordersV, (ImVec2){ .x=0, .y=0 }, 0 ))
		{
			
			igTableSetupColumn("Tick", 0, 0, 0 );
			for (int i = 0; i < game->info->numPlayers; i++) {
				char buff[16];
				sprintf(buff, "P%d", i + 1);
				igTableSetupColumn(buff, 0, 0, 0);
			}
			igTableHeadersRow();

			int numCmds = CmdList_Size( &(server->cmdList) );
			for (int row = 0; row < numCmds; row++)
			{
				igTableNextRow( 0, 0.0f );

				igTableSetColumnIndex(0);
				igText("999" );

				for (int column = 0; column < game->info->numPlayers; column++)
				{
					igTableSetColumnIndex(column+1);
					igText("????", row, column);
				}
			}
			igEndTable();
		}
		igEnd();
	}
}


static void frame(void) {
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale(),
    });

    SlimeGame *game = &server.game;

    // handle tick
    SlimeServer_Update( &server, sapp_frame_duration() );
    
    // draw gamestate
    draw_gamestate();


    /*=== UI CODE STARTS HERE ===*/

	ShowMainMenubar();

	if (guiState.showPlayerWindow)
	{
		ShowPlayerWindow( game );
	}

	if (guiState.showCmdListWindow)
	{
		ShowCommandListWindow(game);
	}

    igSetNextWindowPos((ImVec2){20,20}, ImGuiCond_FirstUseEver, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){400, 200}, ImGuiCond_FirstUseEver);
    
    igBegin("Slimecraft Server GUI", 0, ImGuiWindowFlags_None);


	if ( igCollapsingHeader_BoolPtr("View", NULL, ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen ))
	{
		igColorEdit3("Background", &state.pass_action.colors[0].value.r,
			ImGuiColorEditFlags_None);

		igSliderFloat2("Position", (float*) & (state.cam_center), 0, state.map_size, "%2.0f", ImGuiSliderFlags_None);
		igSliderFloat("Zoom", &(state.cam_zoom), 1.0f,
			state.map_size * 1.5, "%2.0f", ImGuiSliderFlags_None);

		bool doFocus = (state.hFocusUnit != 999);

		igCheckbox("Focus Unit", &doFocus);
		if ((doFocus) && (state.hFocusUnit < game->info->numPlayers)) {
			state.hFocusUnit = SlimeGame_GetUnitByIndex(game, 0);

			SimVec2 pos = SlimeGame_GetUnitPosition(game, state.hFocusUnit);
			state.cam_center = (sgp_vec2) { pos.x, pos.y };
		}
		else {
			state.hFocusUnit = 999;
		}

	}
    igEnd();

    /*=== UI CODE ENDS HERE ===*/

    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());

    // Dispatch all draw commands to Sokol GFX.
    sgp_flush();
    // Finish a draw command queue, clearing it.
    sgp_end();
    
    // Render IMGUI
    simgui_render();

    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {

	SlimeServer_Teardown(&server);

    simgui_shutdown();
    sgp_shutdown();
    sg_shutdown();
}

static void event(const sapp_event* ev) {
    simgui_handle_event(ev);

	switch( ev->type )
	{
	case SAPP_EVENTTYPE_KEY_DOWN:
		
		if (ev->key_code == SAPP_KEYCODE_C)
		{
			// Push on a synthetic command
			Command cmd;
			cmd.cmdType = Command_MOVE;
			cmd.move.unit = 0;
			cmd.move.targetX = 0.0f;
			cmd.move.targetY = 0.0f;
			CmdList_PushCommandForPlayer(&(server.cmdList), server.currentTick + 2, 0, cmd);
		}

		if (ev->key_code == SAPP_KEYCODE_B)
		{
			if (!BotsFull()) {
				BotPlayerInfo *bot = AddBotPlayer();
				if (bot) {
					char buff[16];
					sprintf( buff, "Bot%d", state.numBotPlayers);
					guiState.playerName[bot->playerId] = strdup(buff);
				}
			} else
			{
				printf("Player List is full!\n");
			}
		}
		break;
	}
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
		.init_cb = init,
			.frame_cb = frame, 
        .cleanup_cb = cleanup,
        .event_cb = event,
        .window_title = "SlimeServGUI",
        .width = 800,
        .height = 600,
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
