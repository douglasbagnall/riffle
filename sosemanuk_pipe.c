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

#include "misc.h"
//#include "random_helpers.h"
#include "sosemanuk-clean/sosemanuk.h"

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
rng_init(sosemanuk_run_context *ctx, u32 s)
{
    int i;
    if (s == 0)
        s = 4357;   /* the default seed is 4357, following MT */
    u8 seed[KEY_BYTES + IV_BYTES];
    for (i = 0; i < KEY_BYTES + IV_BYTES; i++){
        s = ((69069 * s) + 1);
        seed[i] = (s >> 24) ^ (s >> 16) ^ (s >> 8);
    }
    sosemanuk_key_context kc;
    sosemanuk_schedule(&kc, seed, KEY_BYTES);
    sosemanuk_init(ctx, &kc, NULL, 0);  /*no IV */
}

int main(int argc, char *argv[]){
    sosemanuk_run_context ctx;
    size_t UNUSED moved;
    u8 *bytes = mmap(NULL, BUFFER_BYTES, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#if VMSPLICE
    struct iovec iov;
    iov.iov_base = bytes;
    iov.iov_len = BUFFER_BYTES;
#endif
    rng_init(&ctx, 3);
    for(;;){
	sosemanuk_prng(&ctx, bytes, BUFFER_BYTES);
#if VMSPLICE
        moved = vmsplice(1, &iov, 1, 0);
#else
        moved = write(1, bytes, BUFFER_BYTES);
#endif
        //debug("%x %x %x %x, wrote %lu\n", bytes[0], bytes[1], bytes[2], bytes[3], moved);
    }
    return 0;
}
