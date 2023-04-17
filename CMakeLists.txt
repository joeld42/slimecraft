cmake_minimum_required(VERSION 3.0)
project(slimecraft)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()

#=== Library: ENET
include( ${PROJECT_SOURCE_DIR}/enet/CMakeLists.txt)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set ( ENET_LIBS enet winmm ws2_32)
else()
set ( ENET_LIBS enet)
endif()


#=== LIBRARY: cimgui + Dear ImGui
add_library(cimgui STATIC 
    cimgui/cimgui.cpp 
    cimgui/cimgui.h 
    cimgui/imgui/imgui.cpp
    cimgui/imgui/imgui.h
    cimgui/imgui/imgui_widgets.cpp 
    cimgui/imgui/imgui_draw.cpp 
    cimgui/imgui/imgui_tables.cpp
    cimgui/imgui/imgui_demo.cpp)
target_include_directories(cimgui INTERFACE cimgui)

#=== LIBRARY: sokol
# add headers to the the file list because they are useful to have in IDEs
set(SOKOL_HEADERS
    sokol/sokol_gfx.h
    sokol/sokol_app.h
    sokol/sokol_imgui.h
    sokol/sokol_glue.h
    sokol/sokol_gp.h
    )


if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set( MATHLIB m )
    add_library(sokol STATIC sokol/sokol.c ${SOKOL_HEADERS})
    target_compile_options(sokol PRIVATE -x objective-c)
    target_link_libraries(sokol PUBLIC
        "-framework QuartzCore"
        "-framework Cocoa"
        "-framework MetalKit"
        "-framework Metal")
else()
    add_library(sokol STATIC sokol/sokol.c ${SOKOL_HEADERS})
    if (CMAKE_SYSTEM_NAME STREQUAL Linux)
        target_link_libraries(sokol INTERFACE X11 Xi Xcursor GL dl m)
        target_link_libraries(sokol PUBLIC Threads::Threads)
    endif()
endif()
target_link_libraries(sokol PUBLIC cimgui)
target_include_directories(sokol INTERFACE sokol)

#=== EXECUTABLE: sokol+cimgui demo
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_executable(demo WIN32 demo.c)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT demo)
else()
    add_executable(demo demo.c)
endif()
target_link_libraries(demo sokol)

#=== Slimecraft test app
set ( SLIMESERV_SRC
    gamestate.h
    gamestate.c
    common.h
    common.c
    cmdlist.h
    cmdlist.c
    network.h
    network.c
    slimeserver.h
    slimeserver.c
    slimeclient.h
    slimeclient.c
	botplayer.h
	botplayer.c
    )


add_executable( slimeserv 
                ${SLIMESERV_SRC}
                slimeserv_cmdline.c )
target_link_libraries(slimeserv ${ENET_LIBS} ${MATHLIB} )

add_executable( slimeserv_gui WIN32
                ${SLIMESERV_SRC}
                slimeserv_gui.c )
target_link_libraries(slimeserv_gui sokol ${ENET_LIBS} ${MATHLIB} )

add_executable( slimeserv_testclient 
                ${SLIMESERV_SRC}
                slimetestclient.c )
target_link_libraries(slimeserv_testclient ${ENET_LIBS} ${MATHLIB} )

# Extra executables == sokol GP samples
add_executable( gp_prims WIN32 sokol_gp/samples/sample-primitives.c )
target_link_libraries(gp_prims sokol ${MATHLIB} )

add_executable( gp_blend WIN32 sokol_gp/samples/sample-blend.c )
target_include_directories( gp_blend PRIVATE sokol_gp/shaders )
target_link_libraries(gp_blend sokol ${MATHLIB} )

add_executable( gp_sdf WIN32 sokol_gp/samples/sample-sdf.c )
target_include_directories( gp_sdf PRIVATE sokol_gp/shaders )
target_link_libraries(gp_sdf sokol ${MATHLIB} )

add_executable( gp_rect WIN32 sokol_gp/samples/sample-rectangle.c )
target_include_directories( gp_rect PRIVATE sokol_gp/shaders )
target_link_libraries(gp_rect sokol ${MATHLIB} )

add_executable( gp_framebuffer WIN32 sokol_gp/samples/sample-framebuffer.c )
target_include_directories( gp_framebuffer PRIVATE sokol_gp/shaders )
target_link_libraries(gp_framebuffer sokol ${MATHLIB} )

add_executable( gp_effect WIN32 sokol_gp/samples/sample-effect.c )
target_include_directories( gp_effect PRIVATE stb_image sokol_gp/shaders )
target_link_libraries(gp_effect sokol ${MATHLIB} )

# add_executable( gp_bench sokol_gp/samples/sample-bench.c )
# target_include_directories( gp_effect PRIVATE sokol_gp/shaders )
# target_link_libraries(gp_bench sokol ${MATHLIB} )


# Emscripten-specific linker options
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    # use our own minimal shell.html
    target_link_options(demo PRIVATE --shell-file ../sokol/shell.html)
    # WASM+JS size optimizations
    target_link_options(demo PRIVATE -sNO_FILESYSTEM=1 -sASSERTIONS=0 -sMALLOC=emmalloc --closure=1)
endif()

# explicitly strip dead code
if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(demo PRIVATE LINKER:-dead_strip)
endif()

# this hack removes the xxx-CMakeForceLinker.cxx dummy file
set_target_properties(demo PROPERTIES LINKER_LANGUAGE C)
set_target_properties(gp_prims PROPERTIES LINKER_LANGUAGE C)
