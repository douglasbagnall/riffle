/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * Part of Riffle, a collection of random number generators
 *
 * This is an experimental un-tested generator based on using the
 * non-cryptographic hash MurmurHash3 in CTR mode.  It uses 96 bits of key and
 * a 64 bit counter generating 2 doubles per round (so 2 ** 65 outputs per
 * key).  It contains code extracted from
 * http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp,
 * which says:
 *
 *    MurmurHash3 was written by Austin Appleby, and is placed in the public
 *    domain. The author hereby disclaims copyright to this source code.
 *
 *  Thanks, Austin Appleby!
 *-----------------------------------------------------------------------------
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
#include <string.h>

#define SEED_BYTES (128 / 8)
#define HASH_SIZE (128 / 8)

typedef struct {
    u64 state[2];
    double numbers[2];
    unsigned int index;
    u32 key;
} rng_context;

/* Here start extracts from MurmurHash.cpp, slightly modified (gone: C++isms,
  untestable MSVC ifdefs, hash flexibility; indentation changed per python
  convention)x */

inline uint32_t rotl32 ( uint32_t x, int8_t r ){
    return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64 ( uint64_t x, int8_t r ){
    return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)


//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

FORCE_INLINE uint64_t fmix ( uint64_t k ){
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;
    return k;
}

/* A simplified version of MurmurHash: the key length is known (128), so some
   looping and conditional code can go */

static inline void
MurmurHash3_x64_128 (const u64 state[2],
		     const uint32_t seed, u64 out[2])
{
    uint64_t h1 = seed;
    uint64_t h2 = seed;

    const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    //----------
    // body is always 128 bits.

    uint64_t k1 = state[0];
    uint64_t k2 = state[1];

    k1 *= c1;
    k1 = ROTL64(k1,31);
    k1 *= c2;
    h1 ^= k1;
    h1 = ROTL64(h1,27);
    h1 += h2;
    h1 = h1 * 5+0x52dce729;
    k2 *= c2;
    k2  = ROTL64(k2,33);
    k2 *= c1;
    h2 ^= k2;
    h2 = ROTL64(h2,31);
    h2 += h1;
    h2 = h2 * 5+0x38495ab5;

    //----------
    // finalization

    h1 ^= 16;
    h2 ^= 16;
    h1 += h2;
    h2 += h1;

    h1 = fmix(h1);
    h2 = fmix(h2);

    h1 += h2;
    h2 += h1;

    out[0] = h1;
    out[1] = h2;
}
/*end of Murmurhash extract */


/* random_random return a double in the range [0, 1).*/

static inline double
rng_double(rng_context *ctx)
{
    if (ctx->index == 1){
	ctx->state[1]++;
	ctx->index = 0;
	MurmurHash3_x64_128(ctx->state, ctx->key, (u64 *)ctx->numbers);
	doubleise_u64_buffer((u64 *)ctx->numbers, 2);
    }
    double d = ctx->numbers[ctx->index];
    ctx->index++;
    return d - 1.0;
}

static inline void
rng_seed(rng_context *ctx, u8* seed, size_t len)
{
    u64 *seed64 = (u64*)seed;
    ctx->state[0] = seed64[0];
    ctx->state[1] = 0;
    ctx->key = (u32)seed64[1];
    ctx->index = 1;
}


static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    for (;;){
	if (len < HASH_SIZE){
	    if (len){
		MurmurHash3_x64_128(ctx->state, ctx->key, (u64*)ctx->numbers);
		ctx->state[1]++;
	    }
	    return;
	}
	MurmurHash3_x64_128(ctx->state, ctx->key, (u64*)bytes);
	ctx->state[1]++;
	bytes += HASH_SIZE;
	len -= HASH_SIZE;
    }
}
