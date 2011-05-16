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

#include <config.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include "dSFMT/dSFMT.h"

#define MODULE_NAME_(x) dSFMT ## x
#define MODULE_NAME__(x) MODULE_NAME_(x)
#define MODULE_NAME MODULE_NAME__(DSFMT_MEXP)

#define SEED_BYTES 16

#include "misc.h"
#include "emitter.h"

#ifndef BUFFER_BYTES
#define BUFFER_BYTES 4096
#endif
#if BUFFER_BYTES & 3
#error  dsfmt_genrand_uint32 makes 4 bytes at a time.
#endif
#define BUFFER_U32S (BUFFER_BYTES / sizeof(u32))


int main(int argc, char *argv[]){
    parse_args(argc, argv);
    size_t remaining = option_bytes;
    u8 *bytes = mmap(NULL, BUFFER_BYTES, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    dsfmt_t dsfmt;
    /* seed */
    u8 seed[SEED_BYTES];
    seed_from_uint(seed, sizeof(seed), option_seed);
    dsfmt_init_by_array(&dsfmt, (u32 *)seed, sizeof(seed) / sizeof(u32));
    /* run */
    u32 *words = (u32 *)bytes;
    uint i;
    for(;option_bytes == 0 || remaining >= BUFFER_BYTES;){
        for (i = 0; i < BUFFER_U32S; i++){
	    words[i] = dsfmt_genrand_uint32(&dsfmt);
	}
        remaining -= write(1, bytes, BUFFER_BYTES);
    }
    if (remaining){
        for (i = 0; i < 1 + remaining / sizeof(u32); i++){
	    words[i] = dsfmt_genrand_uint32(&dsfmt);
	}
        remaining = write(1, bytes, remaining);
    }
    return 0;
}
