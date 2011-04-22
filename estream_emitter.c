/*
 * Copyright (C) 2011 Douglas Bagnall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 */

#include <config.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <unistd.h>

#include "ecrypt-sync.h"
#include "misc.h"
#include "emitter.h"

#ifndef MODULE_NAME
#error define MODULE_NAME and possibly BUFFER_BYTES, KEY_BYTES, and IV_BYTES
#endif
#if ! BUFFER_BYTES + 0
#undef BUFFER_BYTES
#define BUFFER_BYTES 4096
#endif
#if ! KEY_BYTES + 0
#undef KEY_BYTES
#define KEY_BYTES (128 / 8)
#endif
#if ! IV_BYTES + 0
#undef IV_BYTES
#define IV_BYTES (64 / 8)
#endif

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)


/*set the state (integer only), using an LCG
borrowed from mt19937 */
static void
rng_init(ECRYPT_ctx *ctx, u32 s)
{
    u8 seed[KEY_BYTES + IV_BYTES];
    seed_from_uint(seed, sizeof(seed), s);
    ECRYPT_keysetup(ctx, seed, KEY_BYTES * 8, IV_BYTES * 8);
    ECRYPT_ivsetup(ctx, seed + KEY_BYTES);
}

int main(int argc, char *argv[]){
    parse_args(argc, argv);
    ECRYPT_ctx ctx;
    u8 *bytes = mmap(NULL, BUFFER_BYTES, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    rng_init(&ctx, option_seed);
    size_t remaining = option_bytes;
    for(;option_bytes == 0 || remaining >= BUFFER_BYTES;){
        ECRYPT_keystream_bytes(&ctx,
                               bytes,
                               BUFFER_BYTES);
        remaining -= write(1, bytes, BUFFER_BYTES);
    }

    if (remaining){
        ECRYPT_keystream_bytes(&ctx,
                               bytes,
                               BUFFER_BYTES);
        remaining = write(1, bytes, remaining);
    }
    return 0;
}
