#include "phelix/ecrypt-sync-ae.h"
#include "misc.h"
#include <string.h>

#ifndef BUFFER_DOUBLES
#if RESCUE_BITS
#define RESCUED_DOUBLES 16
#define BUFFER_DOUBLES (RESCUED_DOUBLES * 5)
#else
#define BUFFER_DOUBLES 64
#define RESCUED_DOUBLES 0
#endif
#endif

#define KEY_BYTES (128 / 8)
#define IV_BYTES (128 / 8)
#define MAC_BYTES (64 / 8)
#define SEED_BYTES (KEY_BYTES + IV_BYTES)

typedef struct {
    ECRYPT_AE_ctx ctx;
    double numbers[BUFFER_DOUBLES + RESCUED_DOUBLES];
    u32 index;
} rng_context;


static inline double
rng_double(rng_context *ctx)
{
    if (ctx->index >= BUFFER_DOUBLES + RESCUED_DOUBLES){
	ECRYPT_AE_encrypt_bytes(
	    &ctx->ctx,
	    (u8*)ctx->numbers,
	    (u8*)ctx->numbers,
	    BUFFER_DOUBLES * sizeof(double));
	ctx->index = 0;
#if RESCUE_BITS
	doubleise_u64_buffer_with_rescuees((u64 *)ctx->numbers,
					   BUFFER_DOUBLES,
					   (u64 *)ctx->numbers + BUFFER_DOUBLES);
#else
	doubleise_u64_buffer((u64 *)ctx->numbers,
			     BUFFER_DOUBLES);
#endif
    }
    double d = ctx->numbers[ctx->index];
    ctx->index++;
#if DOUBLE_COERCION == COERCE_DSFMT
    return d - 1.0;
#else
    return d;
#endif
}

static inline void
rng_seed(rng_context *ctx, u8* seed, size_t len)
{
    memset(ctx->numbers, 'o', BUFFER_DOUBLES + RESCUED_DOUBLES);
    ECRYPT_AE_keysetup(&ctx->ctx, seed, KEY_BYTES * 8, IV_BYTES * 8, MAC_BYTES * 8);
    ECRYPT_AE_ivsetup(&ctx->ctx, seed + KEY_BYTES);
    ctx->index = BUFFER_DOUBLES + RESCUED_DOUBLES;
}

static inline void
rng_bytes(rng_context *ctx, u8 *bytes, size_t len)
{
    memset(bytes, 'o', len);
    ECRYPT_AE_encrypt_bytes(&ctx->ctx, bytes, bytes, len);
}
