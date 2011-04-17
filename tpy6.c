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

#define MODULE_NAME tpy6
#define BUFFER_DOUBLES 16
#define KEY_BYTES (128 / 8)
#define IV_BYTES (64 / 8)

typedef struct {
    PyObject_HEAD
    ECRYPT_ctx ctx;
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
	ECRYPT_keystream_bytes(&self->ctx,
			       (u8*)self->numbers,
			       BUFFER_DOUBLES * sizeof(double));
	self->index = 0;
	for (int i = 0; i < BUFFER_DOUBLES; i++){
	    u64 *a = ((u64 *)self->numbers) + i;
	    *a &= DSFMT_LOW_MASK;
	    *a |= DSFMT_HIGH_CONST;
	}
    }
    double d = self->numbers[self->index];
    self->index++;
    return PyFloat_FromDouble(d - 1.0);
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
    memset(seed, '#', KEY_BYTES + IV_BYTES);

    if (extract_seed(arg, seed) != 0){
	return NULL;
    }
    ECRYPT_keysetup(&self->ctx, seed, KEY_BYTES * 8, IV_BYTES * 8);
    ECRYPT_ivsetup(&self->ctx, seed + KEY_BYTES);
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

RANDOM_MODULE_INIT2_ECRYPT(MODULE_NAME)

