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
#include "ccan/isaac/isaac.c"

#define SEED_BYTES 20

typedef isaac_ctx rng_context;

static inline double
rng_double(rng_context *ctx)
{
#if 1
    return isaac_next_double(ctx);
#else
    return dsfmt_int64_to_double(isaac_next_uint64(ctx));
#endif
}

static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    u32 *buffer32 = (u32 *)bytes;
    for (;;){
	if (len < sizeof(u32)) {
	    if (len){
		u32 r = isaac_next_uint32(ctx);
		u8 *rb = (u8 *)&r;
		u8* last = (u8 *)buffer32;
		uint i = 0;
		for (i = 0; i < len; i++){
		    last[i] = rb[i];
		}
	    }
	    return;
	}
	*buffer32 = isaac_next_uint32(ctx);
	buffer32++;
	len -= 4;
    }
}


static inline void
rng_seed(rng_context *ctx, u8* seed, size_t len)
{
    isaac_init(ctx, seed, len);
}

