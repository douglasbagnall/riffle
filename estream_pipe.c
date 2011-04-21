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

#define VMSPLICE 0
#if VMSPLICE
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
#define BUFFER_BYTES 4096
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
    size_t UNUSED moved;
    //u8 bytes[BUFFER_BYTES] __attribute__ ((aligned (16)));
    u8 *bytes = mmap(NULL, BUFFER_BYTES, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#if VMSPLICE
    struct iovec iov;
    iov.iov_base = bytes;
    iov.iov_len = BUFFER_BYTES;
#endif
    rng_init(&ctx, 3);
    for(;;){
        ECRYPT_keystream_bytes(&ctx,
                               bytes,
                               BUFFER_BYTES);
#if VMSPLICE
        moved = vmsplice(1, &iov, 1, 0);
#else
        moved = write(1, bytes, BUFFER_BYTES);
#endif
    }
    return 0;
}
