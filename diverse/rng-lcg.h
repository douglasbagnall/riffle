#include "misc.h"

typedef u32 rng_context;
#define SEED_BYTES sizeof(rng_context)

//x_{n+1} = (a x_n + c) mod m
//with a = 1103515245, c = 12345 and m = 2^31
#define RAND_MUL  1103515245
#define RAND_ADD  12345
#define RAND_MASK 0x7fffffff
#define RAND_NORM  (1.0 / RAND_MASK)
#define RAND_BITS  31

static inline double
rng_double(rng_context *ctx)
{
    *ctx = ((*ctx * RAND_MUL) + RAND_ADD) & RAND_MASK;
    return *ctx * RAND_NORM;
}

static inline void
rng_seed(rng_context *ctx, u8* seed, size_t len)
{
    *ctx = *(rng_context *)seed;
}

static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    unsigned int i;
    for (i = 0; i < len; i++){
	*ctx = ((*ctx * RAND_MUL) + RAND_ADD) & RAND_MASK;
	bytes[i] = *ctx >> (RAND_BITS - 8);
    }
}
