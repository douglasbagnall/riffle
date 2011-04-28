/* Based on Python 3.1's original randommodule.c, which was in turn based on
   Takuji Nishimura and Makoto Matsumoto's MT19937 code.  However, the parts
   of randommodule.c that remain in this file are almost certainly not from
   Nishimura and Matsumoto, rather they are the adaptations and boilerplate to
   match Python's Random() interface, and were seemingly mostly written by
   Raymond Hettinger in 2002.
*/

/* ---------------------------------------------------------------*/

#include "Python.h"
#include <time.h>               /* for seeding to current time */
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
    return PyFloat_FromDouble(*d - 1.0);
}

/*
 * The rest is Python-specific code, neither part of, nor derived from, the
 * Twister download.
 */

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *arg = NULL;

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    u8 seed[KEY_BYTES + IV_BYTES];
    memset(seed, '#', sizeof(seed));

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

static PyObject *
random_getstate(RandomObject *self)
{
    return NULL;
}

static PyObject *
random_setstate(RandomObject *self, PyObject *state)
{
    return NULL;
}

RANDOM_CLASS_NEW()

RANDOM_METHODS_STRUCT_NO_GETRANDBITS();

RANDOM_CLASS_DOC(MODULE_NAME);

RANDOM_OBJECT_STRUCT(MODULE_NAME);

RANDOM_MODULE_DOC(MODULE_NAME);

RANDOM_MODULE_STRUCT(MODULE_NAME);

RANDOM_MODULE_INIT2(MODULE_NAME)

