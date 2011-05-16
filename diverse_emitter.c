/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * Part of Riffle, a collection of random number generators
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided "as is", WITHOUT WARRANTY of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. in no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising from,
 * out of or in connection with the software or the use or other dealings in
 * the Software.
 */
/*
 * Copyright (C) 2011 Douglas Bagnall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 */
#ifndef MODULE_NAME
#error define MODULE_NAME and possibly SEED_BYTES
#endif

#include <config.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#if ECRYPT_API
#include "ecrypt-portable.h"
#endif
#include "misc.h"
#include "emitter.h"

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)
#define INCLUDE_NAME_(NAME) diverse/rng-NAME.h
#define INCLUDE_NAME INCLUDE_NAME_(MODULE_NAME)

#include QUOTE(INCLUDE_NAME)

#ifndef BUFFER_BYTES
#define BUFFER_BYTES 4096
#endif


int main(int argc, char *argv[]){
    parse_args(argc, argv);
    size_t remaining = option_bytes;
    u8 *bytes = mmap(NULL, BUFFER_BYTES, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    rng_context ctx;

    /* seed */
    u8 seed[SEED_BYTES];
    seed_from_uint(seed, sizeof(seed), option_seed);
    rng_seed(&ctx, seed, SEED_BYTES);
    
    /* run */
    for(;option_bytes == 0 || remaining >= BUFFER_BYTES;){
	rng_bytes(&ctx, bytes, BUFFER_BYTES);
        remaining -= write(1, bytes, BUFFER_BYTES);
    }
    if (remaining){
	rng_bytes(&ctx, bytes, remaining);
        remaining = write(1, bytes, remaining);
    }
    return 0;
}
