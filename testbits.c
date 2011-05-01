/* Based on Python 3.1's original randommodule.c, which was in turn based on
   Takuji Nishimura and Makoto Matsumoto's MT19937 code.  However, the parts
   of randommodule.c that remain in this file are almost certainly not from
   Nishimura and Matsumoto, rather they are the adaptations and boilerplate to
   match Python's Random() interface, and were seemingly mostly written by
   Raymond Hettinger in 2002.
*/

/* ---------------------------------------------------------------*/

#include "Python.h"
#include <stdint.h>
#include "random_helpers.h"

#define MODULE_NAME testbits

#define N 624
typedef struct {
    PyObject_HEAD
    uint32_t index;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)

#define TWO12 ((u64)1 << 12)
#define TWO52 ((u64)1 << 52)
#define TWOX(x) ((u64)1 << (x))
#define TWOX_1(x) (((u64)1 << (x)) - 1)

#include "testbits.h"

static PyObject *
random_random(RandomObject *self)
{
    union{
	u64 i;
	double d;
    } x;
    double d;
    if (self->index >= sizeof(values) / sizeof(values[0])){
	self->index = 0;
    }
    x.i = values[self->index];
#if RESCUE_BITS
    x.i = x.i >> 12;
#endif
#if DOUBLE_COERCION == COERCE_DSFMT || ! defined DOUBLE_COERCION
    DSFMT_INT64_TO_DOUBLE(x.i);
    d = x.d;
    d -= 1.0;
#elif DOUBLE_COERCION == COERCE_LDEXP
    d = ldexp((double)x.i, -64);
#elif DOUBLE_COERCION == COERCE_MUL
    d = (double)x.i * U64_TO_DOUBLE;
#endif
    self->index++;
    return PyFloat_FromDouble(d);
}

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    /*ignore seed. later it could pick the sets of numbers or something */
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

