#include "common.h"


void RNG_InitWithSeed( RNGState *rng, uint32_t seed ) {
    rng->z1 = seed;
    rng->z2 = seed;
    rng->z3 = seed;
    rng->z4 = seed;
}

void RNG_Init( RNGState *rng ) {
    RNG_InitWithSeed( rng, 12345 );
}

// lfsr113_Bits
u32 RNG_Next( RNGState *rng ) {
   
   u32 z1 = rng->z1;
   u32 z2 = rng->z2;
   u32 z3 = rng->z3;
   u32 z4 = rng->z4;

   u32 b;
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27; 
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;

    rng->z1 = z1;
    rng->z2 = z2;
    rng->z3 = z3;
    rng->z4 = z4;

   return (z1 ^ z2 ^ z3 ^ z4);
}


f32 RNG_NextFloat( RNGState *rng ) {
    u32 val = RNG_Next( rng );
    return (f32)val / (f32)0xFFFFFFFF;
}

f32 RNG_NextFloatRange( RNGState *rng, f32 rangeMin, f32 rangeMax ) {
    u32 val = RNG_Next( rng );
    f32 f = (f32)val / (f32)0xFFFFFFFF;
    return rangeMin + f * (rangeMax - rangeMin);
}