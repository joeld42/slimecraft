#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

//==[ Basic Types ]==========================

typedef uint8_t   u8;
typedef int8_t    i8;
typedef uint16_t  u16;
typedef int16_t   i16;
typedef uint32_t  u32;
typedef int32_t   i32;
typedef uint64_t  u64;
typedef int64_t   i64;
typedef float     f32;
typedef double    f64;
typedef uint8_t   b8;
typedef uint32_t  b32;

//==[ Game Settings ]==========================

// Sim tick is 10 ticks per second
#define SIMTICK_TIME (1.0f/10.0f)
#define SIMTICKS_PER_COMM_TURN (4)

#define MAX_PLAYERS (6)

//===[ Game types ] ======

// handle types, might make struct
typedef u32 HUnit;
typedef u32 Building;

// decls
typedef struct CommandTurn_s CommandTurn;

//==[ Random Number Generator ]==========================

typedef  struct {
    u32 z1;
    u32 z2;
    u32 z3;
    u32 z4;    
} RNGState;

void RNG_InitWithSeed( RNGState *rng, uint32_t seed );
void RNG_Init( RNGState *rng );
u32 RNG_Next( RNGState *rng );
f32 RNG_NextFloat( RNGState *rng );
f32 RNG_NextFloatRange( RNGState *rng, f32 rangeMin, f32 rangeMax );




#endif