/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * Part of Riffle, a collection of random number generators
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided "as is", WITHOUT WARRANTY of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. in no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising from,
 * out of or in connection with the software or the use or other dealings in
 * the Software.
 */

#include "misc.h"
#include "hc128_opt32.h"

#define KEY_BYTES (128 / 8)
#define IV_BYTES (128 / 8)
#define SEED_BYTES (KEY_BYTES + IV_BYTES)

/* DOUBLES_PER_HC128 is determined elsewhere. Don't change. */
#define DOUBLES_PER_HC128 (8)
#define BYTES_PER_HC128 (DOUBLES_PER_HC128 * sizeof(double))

typedef struct {
    HC128_State state;
    u32 index;
} rng_context;

#define hc128_genrand SixteenSteps

static inline double
rng_double(rng_context *ctx)
{
    if (ctx->index >= DOUBLES_PER_HC128){
	hc128_genrand(&ctx->state);
	ctx->index = 0;
	doubleise_u64_buffer((u64 *)ctx->state.keystream,
			     DOUBLES_PER_HC128);
    }
    double *d = ((double *)ctx->state.keystream) + ctx->index;
    ctx->index ++;
#if DOUBLE_COERCION == COERCE_DSFMT
    return *d - 1.0;
#else
    return *d;
#endif
}


static inline void
rng_seed(rng_context *ctx, u8* seed, size_t len)
{
    Initialization(&ctx->state, seed, seed + KEY_BYTES);
    ctx->index = DOUBLES_PER_HC128;
}



static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    for (;;){
	if (len < BYTES_PER_HC128){
	    if (len){
		hc128_genrand(&ctx->state);
		memcpy(bytes, ctx->state.keystream, len);
	    }
	    return;
	}
	hc128_genrand(&ctx->state);
	memcpy(bytes, ctx->state.keystream, BYTES_PER_HC128);
	bytes += BYTES_PER_HC128;
	len -= BYTES_PER_HC128;
    }
}
