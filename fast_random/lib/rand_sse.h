/////////////////////////////////////////////////////////////////////////////
// The Software is provided "AS IS" and possibly with faults.
// Intel disclaims any and all warranties and guarantees, express, implied or
// otherwise, arising, with respect to the software delivered hereunder,
// including but not limited to the warranty of merchantability, the warranty
// of fitness for a particular purpose, and any warranty of non-infringement
// of the intellectual property rights of any third party.
// Intel neither assumes nor authorizes any person to assume for it any other
// liability. Customer will use the software at its own risk. Intel will not
// be liable to customer for any direct or indirect damages incurred in using
// the software. In no event will Intel be liable for loss of profits, loss of
// use, loss of data, business interruption, nor for punitive, incidental,
// consequential, or special damages of any kind, even if advised of
// the possibility of such damages.
//
// Copyright (c) 2003 Intel Corporation
//
// Third-party brands and names are the property of their respective owners
//
///////////////////////////////////////////////////////////////////////////
// Random Number Generation for SSE / SSE2
// Source File
// Version 0.1
// Author Kipp Owens, Rajiv Parikh
////////////////////////////////////////////////////////////////////////



#ifndef RAND_SSE_H
#define RAND_SSE_H

#include "emmintrin.h"
#include <math.h>
#include <stdlib.h>
#include <cstdint>
#ifndef NO_CILK
#include <cilk/cilk.h>
#define ALIGN(X)     __declspec( align(X) )
#else
#define ALIGN(X)     __attribute__ ((aligned(8 * X)))
#endif


//define this if you wish to return values similar to the standard rand();
void srand_sse(__m128i &cur_seed, uint32_t seed);

void rand_sse(__m128i &cur_seed, uint32_t* );

void srand_sse(__m128i &cur_seed, uint32_t seed) {
    cur_seed = _mm_set_epi32(seed, seed + 1, seed, seed + 1);
}

inline void frand_sse(__m128i &cur_seed, float* result) {
    const float max_value = 4294967295.;
    uint32_t rand_uints[4];
    rand_sse(cur_seed, rand_uints);
#pragma simd
    for (int i = 0; i < 4; i++) {
        result[i] = float(rand_uints[i]) / max_value;
    }
}

inline void rand_sse(__m128i &cur_seed, uint32_t* result) {
     __m128i cur_seed_split;
    ALIGN(16) __m128i multiplier;
    ALIGN(16) __m128i adder;
    ALIGN(16) __m128i mod_mask;
    ALIGN(16) __m128i sra_mask;
    ALIGN(16) __m128i sseresult;
    ALIGN(16) static const uint32_t mult[4] =
    { 214013, 17405, 214013, 69069 };

    ALIGN(16) static const uint32_t gadd[4] =
    { 2531011, 10395331, 13737667, 1 };

    ALIGN(16) static const uint32_t mask[4] =
    { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };

    ALIGN(16) static const uint32_t masklo[4] =
    { 0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF };

    adder = _mm_load_si128( (__m128i*) gadd);
    multiplier = _mm_load_si128( (__m128i*) mult);
    mod_mask = _mm_load_si128( (__m128i*) mask);
    sra_mask = _mm_load_si128( (__m128i*) masklo);
    cur_seed_split = _mm_shuffle_epi32( cur_seed, _MM_SHUFFLE( 2, 3, 0, 1 ) );
    cur_seed = _mm_mul_epu32( cur_seed, multiplier );
    multiplier = _mm_shuffle_epi32( multiplier, _MM_SHUFFLE( 2, 3, 0, 1 ) );
    cur_seed_split = _mm_mul_epu32( cur_seed_split, multiplier );
    cur_seed = _mm_and_si128( cur_seed, mod_mask);
    cur_seed_split = _mm_and_si128( cur_seed_split, mod_mask );
    cur_seed_split = _mm_shuffle_epi32( cur_seed_split, _MM_SHUFFLE( 2, 3, 0, 1 ) );
    cur_seed = _mm_or_si128( cur_seed, cur_seed_split );
    cur_seed = _mm_add_epi32( cur_seed, adder);

#ifdef COMPATABILITY
    // Add the lines below if you wish to reduce your results to 16-bit vals...
    sseresult = _mm_srai_epi32(cur_seed, 16);
    sseresult = _mm_and_si128(sseresult, sra_mask);
    _mm_storeu_si128((__m128i*) result, sseresult);
    return;
#endif
    _mm_storeu_si128((__m128i*) result, cur_seed);
    return;
}


inline void frand_sse_array(__m128i &cur_seed, int count, float *out) {
    int set_count = ceil(count / 4.);
    int start_index;
    int i;
    for(i = 0; i < set_count; i++) {
        start_index = i * 4;
        frand_sse(cur_seed, &out[start_index]);
    }
}


inline void rand_sse_array(__m128i &cur_seed, int count, uint32_t *out) {
    int set_count = ceil(count / 4.);
    int start_index;
    int i;
    for(i = 0; i < set_count; i++) {
        start_index = i * 4;
        rand_sse(cur_seed, &out[start_index]);
    }
}


#ifndef NO_CILK
inline void rand_sse_array_cilk(__m128i &cur_seed, int count, uint32_t *out) {
    int set_count = ceil(count / 4.);
    int start_index;
#ifdef __cplusplus
    cilk_for(int i = 0; i < set_count; i++) {
#else
    int i;
    cilk_for(i = 0; i < set_count; i++) {
#endif
        start_index = i * 4;
        rand_sse(cur_seed, &out[start_index]);
    }
}
#endif


inline void rand_array(int count, uint32_t *out) {
    int i;

    for(i = 0; i < count; i++) {
        out[i] = rand();
    }
}


#endif
