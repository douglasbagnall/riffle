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

#include "misc.h"
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

static void
rng_init(sosemanuk_run_context *ctx, u32 s)
{
    u8 seed[KEY_BYTES + IV_BYTES];
    seed_from_uint(seed, sizeof(seed), s);
    sosemanuk_key_context kc;
    sosemanuk_schedule(&kc, seed, KEY_BYTES);
    sosemanuk_init(ctx, &kc, NULL, 0);  /*no IV */
}

int main(int argc, char *argv[]){
    sosemanuk_run_context ctx;
    size_t UNUSED moved;
    u8 *bytes = mmap(NULL, BUFFER_BYTES, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    rng_init(&ctx, 3);
    for(;;){
	sosemanuk_prng(&ctx, bytes, BUFFER_BYTES);
        moved = write(1, bytes, BUFFER_BYTES);
    }
    return 0;
}
