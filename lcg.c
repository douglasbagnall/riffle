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
/* Based on Python 3.1's original randommodule.c, which was in turn based on
   Takuji Nishimura and Makoto Matsumoto's MT19937 code.  However, the parts
   of randommodule.c that remain in this file are almost certainly not from
   Nishimura and Matsumoto, rather they are the adaptations and boilerplate to
   match Python's Random() interface, and were seemingly mostly written by
   Raymond Hettinger in 2002.
*/

#include "Python.h"
#include <time.h>               /* for seeding to current time */
#include "random_helpers.h"

#define MODULE_NAME lcg

#define N 624
typedef struct {
    PyObject_HEAD
    uint32_t state;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)

//x_{n+1} = (a x_n + c) mod m
//with a = 1103515245, c = 12345 and m = 2^31
#define RAND_MUL  1103515245
#define RAND_ADD  12345
#define RAND_MASK 0x7fffffff
#define RAND_NORM  (1.0 / RAND_MASK)

/* random_random return a double in the range [0, 1).
 * For this module, there are only 32 significant bits.
*/
static PyObject *
random_random(RandomObject *self)
{
    self->state = ((self->state * RAND_MUL) + RAND_ADD) & RAND_MASK;
    return PyFloat_FromDouble(self->state * RAND_NORM);
}

static void
init_genrand(RandomObject *self, unsigned long s)
{
    self->state = s & RAND_MASK;
}

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *arg = NULL;

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    /* There is no use in having more than 31 bits of seed, so this is simpler
     * than more sophisiticated generators.  Just use the hash; for integers
     * it tends to be the value anyway.
     */

    long seed;
    if (arg == NULL || arg == Py_None) {
        time_t now;
        time(&now);
	seed = (long)now;
    }
    else{
	seed = PyObject_Hash(arg);
    }

    init_genrand(self, seed);
    Py_INCREF(Py_None);
    return Py_None;
}

RANDOM_DUMMY_STATE_SETTERS()

static PyObject *
random_getrandbits(RandomObject *self, PyObject *args)
{
    int k, i, bytes;
    unsigned long r;
    unsigned char *bytearray;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "i:getrandbits", &k))
        return NULL;

    if (k <= 0) {
        PyErr_SetString(PyExc_ValueError,
                        "number of bits must be greater than zero");
        return NULL;
    }

    bytes = ((k - 1) / 32 + 1) * 4;
    bytearray = (unsigned char *)PyMem_Malloc(bytes);
    if (bytearray == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    /* Fill-out 16 bit words, byte-by-byte to avoid endianness issues
       Skip low order bits, because they're crap.*/
    for (i=0 ; i<bytes ; i+=2, k-=16) {
	self->state = ((self->state * RAND_MUL) + RAND_ADD) & RAND_MASK;
        if (k < 16)
            r >>= (16 - k);
        bytearray[i+0] = (unsigned char) self->state;
        bytearray[i+1] = (unsigned char)(self->state >> 8);
    }

    /* little endian order to match bytearray assignment order */
    result = _PyLong_FromByteArray(bytearray, bytes, 1, 0);
    PyMem_Free(bytearray);
    return result;
}

RANDOM_CLASS_NEW()

RANDOM_METHODS_STRUCT();

RANDOM_CLASS_DOC(MODULE_NAME);

RANDOM_OBJECT_STRUCT(MODULE_NAME);

RANDOM_MODULE_DOC(MODULE_NAME);

RANDOM_MODULE_STRUCT(MODULE_NAME);

RANDOM_MODULE_INIT2(MODULE_NAME)

