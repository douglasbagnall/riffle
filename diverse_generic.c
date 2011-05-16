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
#if ECRYPT_API
#include "ecrypt-portable.h"
#endif
#include "random_helpers.h"

#ifndef MODULE_NAME
#error define MODULE_NAME and possibly SEED_BYTES
#endif


#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

/* Note: ## is not needed because - and . are not valid tokens */
#define INCLUDE_NAME_(NAME) diverse/rng-NAME.h
#define INCLUDE_NAME INCLUDE_NAME_(MODULE_NAME)

#include QUOTE(INCLUDE_NAME)


typedef struct {
    PyObject_HEAD
    rng_context context;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)

/* random_random return a double in the range [0, 1).
*/
static PyObject *
random_random(RandomObject *self)
{
    double d =  rng_double(&self->context);
    return PyFloat_FromDouble(d);
}

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *arg = NULL;
    u8 seed[SEED_BYTES + 4];

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    if (extract_seed(arg, seed, sizeof(seed)) != 0){
	return NULL;
    }

    rng_seed(&self->context, seed, SEED_BYTES);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
random_getrandbits(RandomObject *self, PyObject *args)
{
    int k, bytes;
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

    /*The mt19937 original flipped bytes to ensure the same result on big- and
      little- endian machines.  Not doing that here, because:
      a) it is fiddly
      b) it doesn't really matter (result is equally random)
      c) it is unclear in general where the bytes should be flipped -- the original
         was based on 32 bit words, but some of these generators are 64 bit,
	 8 bit, or amorphous bytestreams.
     */
    rng_bytes(&self->context, bytearray, bytes);
    truncate_bitarray(bytearray, bytes, k);

    result = _PyLong_FromByteArray(bytearray, bytes, 1, 0);
    PyMem_Free(bytearray);
    return result;
}

static PyObject *
random_getstate(RandomObject *self)
{
    PyObject *state;
    char *data = (char *)&self->context;
    Py_ssize_t size = sizeof(self->context);

    state = PyByteArray_FromStringAndSize(data, size);
    return state;
}

static PyObject *
random_setstate(RandomObject *self, PyObject *state)
{
    if (!PyByteArray_Check(state) ||
	PyByteArray_Size(state) != sizeof(self->context)){
        return PyErr_Format(PyExc_TypeError,
			    "state must be a byte array of size %u",
			    sizeof(self->context));
    }
    char *data = PyByteArray_AsString(state);
    memcpy(&self->context, data, sizeof(self->context));

    Py_INCREF(Py_None);
    return Py_None;
}

RANDOM_CLASS_NEW()

RANDOM_METHODS_STRUCT();

RANDOM_CLASS_DOC(MODULE_NAME);

RANDOM_OBJECT_STRUCT(MODULE_NAME);

RANDOM_MODULE_DOC(MODULE_NAME);

RANDOM_MODULE_STRUCT(MODULE_NAME);

RANDOM_MODULE_INIT2(MODULE_NAME)

