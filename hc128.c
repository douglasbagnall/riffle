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

#define USE_HC32_OPT32
#ifdef USE_HC32_OPT32
#include "hc128_opt32.h"
#else
#include "hc128_ref.h"
#endif

#define MODULE_NAME hc128

#ifdef USE_HC32_OPT32

#define DOUBLES_PER_HC128 (8)

typedef struct {
    PyObject_HEAD
    HC128_State state;
    u32 index;
} RandomObject;

#define hc128_genrand SixteenSteps
#else
#error hc128_ref not done yet
#endif

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)



/* Random methods */
/* random_random return a double in the range [0, 1).
*/
static PyObject *
random_random(RandomObject *self)
{
    if (self->index >= DOUBLES_PER_HC128){
	hc128_genrand(&self->state);
	self->index = 0;
	for (int i = 0; i < DOUBLES_PER_HC128; i ++){
	    u64 *a = ((u64 *)self->state.keystream) + i;
	    *a &= DSFMT_LOW_MASK;
	    *a |= DSFMT_HIGH_CONST;
	}
    }
    double *d = ((double *)self->state.keystream) + self->index;
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

    u8 seed[256] = ("abcdefghijklmnopqrstuvwxyz123456"
		    "abcdefghijklmnopqrstuvwxyz123456"
		    "abcdefghijklmnopqrstuvwxyz123456"
		    "abcdefghijklmnopqrstuvwxyz123456"
		    "abcdefghijklmnopqrstuvwxyz123456"
		    "abcdefghijklmnopqrstuvwxyz123456"
		    "abcdefghijklmnopqrstuvwxyz123456"
		    "abcdefghijklmnopqrstuvwxyz123456"
	);
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

    /*this function initialize the state using 128-bit key and 128-bit IV*/
    Initialization(&self->state, seed, seed + 128);

    self->index = DOUBLES_PER_HC128;
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


static PyObject *
random_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    RandomObject *self;
    PyObject *tmp;

    if (type == &Random_Type && !_PyArg_NoKeywords("Random()", kwds))
        return NULL;

    self = (RandomObject *)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;
    tmp = random_seed(self, args);
    if (tmp == NULL) {
        Py_DECREF(self);
        return NULL;
    }
    Py_DECREF(tmp);
    return (PyObject *)self;
}

RANDOM_METHODS_STRUCT_NO_GETRANDBITS();

RANDOM_CLASS_DOC(MODULE_NAME);

RANDOM_OBJECT_STRUCT(MODULE_NAME);

RANDOM_MODULE_DOC(MODULE_NAME);

RANDOM_MODULE_STRUCT(MODULE_NAME);

RANDOM_MODULE_INIT2(MODULE_NAME)
