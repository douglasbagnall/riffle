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
#include "Python.h"
#include "random_helpers.h"
#include "ccan/isaac/isaac.c"

#define SEED_BYTES 20

typedef struct {
    PyObject_HEAD
    struct isaac_ctx context;
} RandomObject;

static inline double
rng_double(RandomObject *self)
{
#if 1
    return isaac_next_double(&self->context);
#else
    return dsfmt_int64_to_double(isaac_next_uint64(&self->context));
#endif
}

static inline void
rng_seed(RandomObject *self, u8* seed, size_t len)
{
    isaac_init(&self->context, seed, len);
}

