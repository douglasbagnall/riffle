
/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * Uses xx-tea encoding as a random number generator.
 *
 * This file was orignally based on Python 3.1's original randommodule.c,
 * which was in turn based on Takuji Nishimura and Makoto Matsumoto's MT19937
 * code and adapted to Python by Raymond Hettinger.  No trace of MT19937
 * remains here, and most of the Python boilerplate has been moved to
 * macros in random_helpers.h.
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

/* ---------------------------------------------------------------*/

#include "Python.h"
#include "random_helpers.h"
#include <stdint.h>

#define MODULE_NAME xxtea

#define BUFFER_DOUBLES (64)
#define BUFFER_U32S (BUFFER_DOUBLES * sizeof(double) / sizeof(u32))
#define KEY_BYTES (128 / 8)
#define IV_BYTES (0 / 8)

typedef struct {
    PyObject_HEAD
    u32 key[4];
    double numbers[BUFFER_DOUBLES];
    u32 index;
    u64 ctr;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)

/* Random methods */

#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (k[(p&3)^e] ^ z)))
#define DELTA 0x9e3779b9

static inline void
btea_enc(u32 *v, unsigned int n, u32 const k[4]) {
    uint32_t y, z, sum;
    unsigned int p, rounds, e;
    rounds = 6 + 52/n;
    sum = 0;
    z = v[n - 1];
    do {
	sum += DELTA;
	e = (sum >> 2) & 3;
	for (p=0; p < n - 1; p++) {
	    y = v[p + 1];
	    z = v[p] += MX;
	}
	y = v[0];
	z = v[n - 1] += MX;
    } while (--rounds);
}


/* random_random return a double in the range [0, 1).
*/
static PyObject *
random_random(RandomObject *self)
{
    if (self->index >= BUFFER_DOUBLES){
 	btea_enc((u32 *) &self->numbers,
		 BUFFER_U32S,
		 self->key
	    );
	self->index = 0;
	doubleise_u64_buffer((u64 *)self->numbers,
			     BUFFER_DOUBLES);
    }
    double *d = self->numbers + self->index;
    self->index ++;
#if DOUBLE_COERCION == COERCE_DSFMT
    return PyFloat_FromDouble(*d - 1.0);
#else
    return PyFloat_FromDouble(*d);
#endif
}

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *arg = NULL;
    u8 seed[KEY_BYTES + IV_BYTES + 4];

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    if (extract_seed(arg, seed, sizeof(seed)) != 0){

	return NULL;
    }

    memcpy(self->key, seed, sizeof(self->key));
    memset(self->numbers, '~', sizeof(self->numbers));
    self->index = BUFFER_DOUBLES;
    self->ctr = 0;
    Py_INCREF(Py_None);
    return Py_None;
}

RANDOM_DUMMY_STATE_SETTERS()

RANDOM_CLASS_NEW()

RANDOM_METHODS_STRUCT_NO_GETRANDBITS();

RANDOM_CLASS_DOC(MODULE_NAME);

RANDOM_OBJECT_STRUCT(MODULE_NAME);

RANDOM_MODULE_DOC(MODULE_NAME);

RANDOM_MODULE_STRUCT(MODULE_NAME);

RANDOM_MODULE_INIT2(MODULE_NAME)

