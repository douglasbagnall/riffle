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
    int i = 0;
    u8 *end = bytes + len;
    u64 *buffer64 = (u64 *)bytes;
    for (;;){
	if (len < sizeof(u64)) {
	    if (len){
		u64 r = isaac64_next_uint64(ctx);
		u8* rb = (u8 *)&r;
		for (i = len; i > 0; i--){
		    end[-i] = rb[-i];
		}
	    }
	    return;
	}
	buffer64[i] = isaac64_next_uint64(ctx);
	i++;
	len -= sizeof(u64);
    }
}
