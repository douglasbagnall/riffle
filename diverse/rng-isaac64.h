#include "misc.h"
#include "ccan/isaac/isaac64.c"

#define SEED_BYTES 20

typedef isaac64_ctx rng_context;

static inline double
rng_double(rng_context *ctx)
{
#if 0
    return isaac64_next_double(ctx);
#else
    return dsfmt_int64_to_double(isaac64_next_uint64(ctx));
#endif
}

static inline void
rng_seed(rng_context *ctx, u8* seed, size_t len)
{
    isaac64_init(ctx, seed, len);
}

static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    u64 *buffer64 = (u64 *)bytes;
    for (;;){
	if (len < sizeof(u64)) {
	    if (len){
		u64 r = isaac64_next_uint64(ctx);
		u8* rb = (u8 *)&r;
		u8* last = (u8 *)buffer64;
		uint i = 0;
		for (i = 0; i < len; i++){
		    last[i] = rb[i];
		}
	    }
	    return;
	}
	*buffer64 = isaac64_next_uint64(ctx);
	buffer64++;
	len -= sizeof(u64);
    }
}
