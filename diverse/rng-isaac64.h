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
