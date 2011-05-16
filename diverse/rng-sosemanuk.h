/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * A wrapper for the SOSEMANUK implementation in sosemanuk-clean/
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
#include "sosemanuk-clean/sosemanuk.h"

#define BUFFER_DOUBLES (16)
#define KEY_BYTES (128 / 8)
#define IV_BYTES (0 / 8)
#define SEED_BYTES (KEY_BYTES + IV_BYTES)

typedef struct {
    sosemanuk_run_context run_ctx;
    double numbers[BUFFER_DOUBLES];
    u32 index;
} rng_context;


static inline double
rng_double(rng_context *ctx)
{
    if (ctx->index >= BUFFER_DOUBLES){
	sosemanuk_prng(&ctx->run_ctx, (u8*)ctx->numbers, BUFFER_DOUBLES * sizeof(double));
	ctx->index = 0;
	doubleise_u64_buffer((u64 *)ctx->numbers,
			     BUFFER_DOUBLES);
    }
    double *d = ctx->numbers + ctx->index;
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
    sosemanuk_key_context kc;
    sosemanuk_schedule(&kc, seed, sizeof(seed));
    sosemanuk_init(&ctx->run_ctx, &kc, NULL, 0);  /*no IV */
    ctx->index = BUFFER_DOUBLES;
}

static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    ctx->index = BUFFER_DOUBLES; /*trigger reload on next use of doubles*/
    sosemanuk_prng(&ctx->run_ctx, bytes, len);
}
