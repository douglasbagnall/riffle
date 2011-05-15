/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * Uses xx-tea encoding as a random number generator.
 *
 * This file was orignally based on Python 3.1's original randommodule.c,
 * which was in turn based on Takuji Nishimura and Makoto Matsumoto's MT19937
 * code and adapted to Python by Raymond Hettinger.  No trace of MT19937
 * remains here, and most of the Python boilerplate has been moved to
 * macros in random_helpers.h.
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
#include <stdint.h>
#include <string.h>

#define BUFFER_DOUBLES (64)
#define BUFFER_BYTES (BUFFER_DOUBLES * sizeof(double))
#define BUFFER_U32S (BUFFER_BYTES / sizeof(u32))
#define SEED_BYTES (128 / 8)

typedef struct {
    u32 key[4];
    double numbers[BUFFER_DOUBLES];
    u32 index;
} rng_context;

#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (k[(p&3)^e] ^ z)))
#define DELTA 0x9e3779b9

static inline void
btea_enc(u32 *v, unsigned int n, u32 const k[4]) {
    uint32_t y, z, sum;
    unsigned int p, rounds, e;
    rounds = 6 + 52/n;
    sum = 0;
    z = v[n - 1];
    do {
	sum += DELTA;
	e = (sum >> 2) & 3;
	for (p=0; p < n - 1; p++) {
	    y = v[p + 1];
	    z = v[p] += MX;
	}
	y = v[0];
	z = v[n - 1] += MX;
    } while (--rounds);
}


static inline double
rng_double(rng_context *ctx)
{
    if (ctx->index >= BUFFER_DOUBLES){
 	btea_enc((u32 *) &ctx->numbers,
		 BUFFER_U32S,
		 ctx->key
	    );
	ctx->index = 0;
	doubleise_u64_buffer((u64 *)ctx->numbers,
			     BUFFER_DOUBLES);
    }
    double *d = ctx->numbers + ctx->index;
    ctx->index++;
#if DOUBLE_COERCION == COERCE_DSFMT
    return *d - 1.0;
#else
    return *d;
#endif
}


static inline void
rng_seed(rng_context *ctx, u8* seed, size_t len)
{
    memcpy(ctx->key, seed, sizeof(ctx->key));
    memset(ctx->numbers, '~', sizeof(ctx->numbers));
    ctx->index = BUFFER_DOUBLES;
}


static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    memset(bytes, 'x', len); /*encrypts in place, so needs known plaintext*/
    for (;;){
	if (len < BUFFER_BYTES) {
	    if (len){
		u32 tmp[BUFFER_U32S];
		memset(tmp, 'x', len);
		btea_enc(tmp,
			 BUFFER_U32S,
			 ctx->key
		    );
		memcpy(bytes, tmp, len);
	    }
	    return;
	}
	btea_enc((u32 *) bytes,
		 BUFFER_U32S,
		 ctx->key
	    );
	bytes += BUFFER_BYTES;
	len -= BUFFER_BYTES;
    }
}
