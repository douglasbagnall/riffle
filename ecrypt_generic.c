/* Based on Python 3.1's original randommodule.c, which was in turn based on
   Takuji Nishimura and Makoto Matsumoto's MT19937 code.  However, the parts
   of randommodule.c that remain in this file are almost certainly not from
   Nishimura and Matsumoto, rather they are the adaptations and boilerplate to
   match Python's Random() interface, and were seemingly mostly written by
   Raymond Hettinger in 2002.
*/

/* ---------------------------------------------------------------*/

#include "Python.h"

#include "ecrypt-sync.h"

#include "random_helpers.h"
#define RESCUE_BITS 1

#ifndef MODULE_NAME
#error define MODULE_NAME and possibly BUFFER_DOUBLES, KEY_BYTES, and IV_BYTES
#endif

#ifndef BUFFER_DOUBLES
#if RESCUE_BITS
#if DOUBLE_COERCION != COERCE_DSFMT
#error RESCUE_BITS is set but DOUBLE_COERCION != COERCE_DSFMT
#endif
#define RESCUED_DOUBLES 16
#define BUFFER_DOUBLES (RESCUED_DOUBLES * 5)
#else
#define BUFFER_DOUBLES 64
#define RESCUED_DOUBLES 0
#endif
#endif

#if ! KEY_BYTES + 0
#undef KEY_BYTES
#define KEY_BYTES (128 / 8)
#endif
#if ! IV_BYTES + 0
#undef IV_BYTES
#define IV_BYTES (64 / 8)
#endif

typedef struct {
    PyObject_HEAD
    ECRYPT_ctx ctx;
    double numbers[BUFFER_DOUBLES + RESCUED_DOUBLES];
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
    if (self->index >= BUFFER_DOUBLES + RESCUED_DOUBLES){
	ECRYPT_keystream_bytes(&self->ctx,
			       (u8*)self->numbers,
			       BUFFER_DOUBLES * sizeof(double));
	self->index = 0;
#if RESCUE_BITS
	doubleise_u64_buffer_with_rescuees((u64 *)self->numbers,
					   BUFFER_DOUBLES,
					   (u64 *)self->numbers + BUFFER_DOUBLES);
#else
	doubleise_u64_buffer((u64 *)self->numbers,
			     BUFFER_DOUBLES);
#endif
    }
    double d = self->numbers[self->index];
    self->index++;
#if DOUBLE_COERCION == COERCE_DSFMT
    return PyFloat_FromDouble(d - 1.0);
#else
    return PyFloat_FromDouble(d);
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
    u8 seed[KEY_BYTES + IV_BYTES];

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    if (extract_seed(arg, seed, sizeof(seed)) != 0){
	return NULL;
    }
    ECRYPT_keysetup(&self->ctx, seed, KEY_BYTES * 8, IV_BYTES * 8);
    ECRYPT_ivsetup(&self->ctx, seed + KEY_BYTES);
    self->index = BUFFER_DOUBLES + RESCUED_DOUBLES;
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

RANDOM_MODULE_INIT2_ECRYPT(MODULE_NAME)

