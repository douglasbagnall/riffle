/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * A wrapper for the SOSEMANUK implementation in sosemanuk-clean/
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
#include "sosemanuk-clean/sosemanuk.h"

#define MODULE_NAME sosemanuk

#define BUFFER_DOUBLES (16)
#define KEY_BYTES (128 / 8)
#define IV_BYTES (0 / 8)

typedef struct {
    PyObject_HEAD
    sosemanuk_run_context run_ctx;
    double numbers[BUFFER_DOUBLES];
    u32 index;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)

/* Random methods */

/* random_random return a double in the range [0, 1).
*/
static PyObject *
random_random(RandomObject *self)
{
    if (self->index >= BUFFER_DOUBLES){
	sosemanuk_prng(&self->run_ctx, (u8*)self->numbers, BUFFER_DOUBLES * sizeof(double));
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

    sosemanuk_key_context kc;
    sosemanuk_schedule(&kc, seed, sizeof(seed));
    sosemanuk_init(&self->run_ctx, &kc, NULL, 0);  /*no IV */
    self->index = BUFFER_DOUBLES;
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

