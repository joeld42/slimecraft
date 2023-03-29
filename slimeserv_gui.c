
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

// Game includes
#include "gamestate.h"
#include "slimeserver.h"
#include "slimeclient.h"

ImColor playerCol[4] = {
    { 0.94, 0.02, 0.51, 1.0 },
    { 0.02, 0.84, 0.95, 1.0 },
    { 0.65, 0.96, 0.01, 1.0 },
    { 0.95, 0.79, 0.01, 1.0 }
};

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
} state;

SlimeServer server;


static void init(void) {
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
    igSetNextWindowPos((ImVec2){10,10}, ImGuiCond_Once, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){400, 200}, ImGuiCond_Once);
    
    igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_None);
    igColorEdit3("Background", &state.pass_action.colors[0].value.r, 
    ImGuiColorEditFlags_None);
    
    igSliderFloat2( "Position", (float*)&(state.cam_center), 0, state.map_size, "%2.0f", ImGuiSliderFlags_None );
    igSliderFloat( "Zoom", &(state.cam_zoom), 1.0f, 
                state.map_size * 1.5, "%2.0f", ImGuiSliderFlags_None );

    bool doFocus = (state.hFocusUnit != 999);
    igCheckbox( "Focus Unit", &doFocus );
    if (doFocus) {
        state.hFocusUnit = SlimeGame_GetUnitByIndex( game, 0 );

        SimVec2 pos = SlimeGame_GetUnitPosition( game, state.hFocusUnit );
        state.cam_center = (sgp_vec2){ pos.x, pos.y };
    } else {
        state.hFocusUnit = 999;
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
    simgui_shutdown();
    sgp_shutdown();
    sg_shutdown();
}

static void event(const sapp_event* ev) {
    simgui_handle_event(ev);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .window_title = "SlimeServ",
        .width = 800,
        .height = 600,
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
