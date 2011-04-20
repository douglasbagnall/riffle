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

#define vmsplice 0
#if vmsplice
#include <fcntl.h>
#include <sys/uio.h>
#else
#include <unistd.h>
#endif

#include "ecrypt-sync.h"
#include "misc.h"

#ifndef MODULE_NAME
#error define MODULE_NAME and possibly BUFFER_BYTES, KEY_BYTES, and IV_BYTES
#endif
#ifndef BUFFER_BYTES
#define BUFFER_BYTES 1024
#endif
#ifndef KEY_BYTES
#define KEY_BYTES (128 / 8)
#endif
#ifndef IV_BYTES
#define IV_BYTES (64 / 8)
#endif

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)


/*set the state (integer only), using an LCG
borrowed from mt19937 */
static void
rng_init(ECRYPT_ctx *ctx, u32 s)
{
    int i;
    if (s == 0)
        s = 4357;   /* the default seed is 4357, following MT */
    u8 seed[KEY_BYTES + IV_BYTES];
    for (i = 0; i < KEY_BYTES + IV_BYTES; i++){
        s = ((69069 * s) + 1);
        seed[i] = (s >> 24) ^ (s >> 16) ^ (s >> 8);
    }
    ECRYPT_keysetup(ctx, seed, KEY_BYTES * 8, IV_BYTES * 8);
    ECRYPT_ivsetup(ctx, seed + KEY_BYTES);
}

int main(int argc, char *argv[]){
    ECRYPT_ctx ctx;
    u8 bytes[BUFFER_BYTES] __attribute__ ((aligned (16)));
#if VMSPLICE
    struct iovec iov = { bytes, BUFFER_BYTES };
#endif
    rng_init(&ctx, 3);
    //u32 i;
    //for(i = 0; i < 256; i++){
    for(;;){
        ECRYPT_keystream_bytes(&ctx,
                               bytes,
                               BUFFER_BYTES);
#if VMSPLICE
        size_t moved = vmsplice(1, &iov, 1, 0);
#else
        size_t moved = write(1, bytes, BUFFER_BYTES);
#endif
        //debug("moved %lu bytes\n", moved);
    }
    return 0;
}
