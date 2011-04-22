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
#include "ccan/isaac/isaac64.h"

#define MODULE_NAME isaac64

#define BUFFER_DOUBLES (16)

typedef struct {
    PyObject_HEAD
    struct isaac64_ctx context;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)

/* Random methods */

/* random_random return a double in the range [0, 1).
*/
static PyObject *
random_random(RandomObject *self)
{
#if 0
    double d =  isaac64_next_double(&self->context);
#else
    union{
	u64 i;
	double d;
    } x;
    x.i = isaac64_next_uint64(&self->context);
    x.i &= DSFMT_LOW_MASK;
    x.i |= DSFMT_HIGH_CONST;
    return PyFloat_FromDouble(x.d - 1.0);
#endif
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

    u8 seed[20] = "12345678901234567890";
    if (arg == NULL || arg == Py_None) {
	/*XXX should use urandom */
        time_t now;
        time(&now);
	snprintf((char *)seed, sizeof(seed), "%lx%p%p", now, &now, &self);
    }
    else if (PyObject_CheckReadBuffer(arg)){
	const void *buffer;
	Py_ssize_t buffer_len;
	if (PyObject_AsReadBuffer(arg, &buffer, &buffer_len)){
	    return NULL;
	}
	initialise_state(seed, sizeof(seed), (u8*)buffer, buffer_len);
    }
    else {
	/*use python hash. it would be possible, but perhaps surprising to
	  use the string representation */
	long hash = PyObject_Hash(arg);
	//debug("seeding with hash %ld\n", hash);
	snprintf((char *)seed, sizeof(seed), "%ld", hash);
    }
    /*now we have a seed */

    isaac64_init(&self->context, seed, sizeof(seed));

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

