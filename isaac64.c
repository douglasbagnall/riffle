/* Based on Python 3.1's original randommodule.c, which was in turn based on
   Takuji Nishimura and Makoto Matsumoto's MT19937 code.  However, the parts
   of randommodule.c that remain in this file are almost certainly not from
   Nishimura and Matsumoto, rather they are the adaptations and boilerplate to
   match Python's Random() interface, and were seemingly mostly written by
   Raymond Hettinger in 2002.
*/

/* ---------------------------------------------------------------*/

#include "Python.h"
#include "random_helpers.h"
#include "ccan/isaac/isaac64.h"
#ifndef KEY_BYTES
#define KEY_BYTES (160 / 8)
#endif
#ifndef IV_BYTES
#define IV_BYTES (0 / 8)
#endif

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
    DSFMT_INT64_TO_DOUBLE(x.i);
#if DOUBLE_COERCION == COERCE_DSFMT
    return PyFloat_FromDouble(x.d - 1.0);
#else
    return PyFloat_FromDouble(x.d);
#endif
#endif
}

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *arg = NULL;
    u8 seed[KEY_BYTES + IV_BYTES];

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    if (extract_seed(arg, seed, sizeof(seed)) != 0){
	return NULL;
    }

    isaac64_init(&self->context, seed, sizeof(seed));
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

