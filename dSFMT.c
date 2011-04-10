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
#include "dSFMT/dSFMT.h"
#include "random_helpers.h"

#define MODULE_NAME dSFMT

#define N 624
typedef struct {
    PyObject_HEAD
    dsfmt_t dsfmt;
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)


/* Random methods */

/* random_random return a double in the range [0, 1).
*/
static PyObject *
random_random(RandomObject *self)
{
    double a = dsfmt_genrand_close_open(&self->dsfmt);
    return PyFloat_FromDouble(a);
}

/* initializes mt[N] with a seed */
static void
init_genrand(RandomObject *self, unsigned long s)
{
    dsfmt_init_gen_rand(&self->dsfmt, s);
}


/*
 * The rest is Python-specific code, neither part of, nor derived from, the
 * Twister download.
 */

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *result = NULL;            /* guilty until proved innocent */
    PyObject *masklower = NULL;
    PyObject *thirtytwo = NULL;
    PyObject *n = NULL;
    uint32_t *key = NULL;
    unsigned long keymax;               /* # of allocated slots in key */
    unsigned long keyused;              /* # of used slots in key */
    int err;

    PyObject *arg = NULL;

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

    if (arg == NULL || arg == Py_None) {
        time_t now;

        time(&now);
        init_genrand(self, (unsigned long)now);
        Py_INCREF(Py_None);
        return Py_None;
    }
    /* If the arg is an int or long, use its absolute value; else use
     * the absolute value of its hash code.
     */
    if (PyLong_Check(arg))
        n = PyNumber_Absolute(arg);
    else {
        long hash = PyObject_Hash(arg);
        if (hash == -1)
            goto Done;
        n = PyLong_FromUnsignedLong((unsigned long)hash);
    }
    if (n == NULL)
        goto Done;

    /* Now split n into 32-bit chunks, from the right.  Each piece is
     * stored into key, which has a capacity of keymax chunks, of which
     * keyused are filled.  Alas, the repeated shifting makes this a
     * quadratic-time algorithm; we'd really like to use
     * _PyLong_AsByteArray here, but then we'd have to break into the
     * long representation to figure out how big an array was needed
     * in advance.
     */
    keymax = 8;         /* arbitrary; grows later if needed */
    keyused = 0;
    key = (uint32_t *)PyMem_Malloc(keymax * sizeof(*key));
    if (key == NULL)
        goto Done;

    masklower = PyLong_FromUnsignedLong(0xffffffffU);
    if (masklower == NULL)
        goto Done;
    thirtytwo = PyLong_FromLong(32L);
    if (thirtytwo == NULL)
        goto Done;
    while ((err=PyObject_IsTrue(n))) {
        PyObject *newn;
        PyObject *pychunk;
        unsigned long chunk;

        if (err == -1)
            goto Done;
        pychunk = PyNumber_And(n, masklower);
        if (pychunk == NULL)
            goto Done;
        chunk = PyLong_AsUnsignedLong(pychunk);
        Py_DECREF(pychunk);
        if (chunk == (unsigned long)-1 && PyErr_Occurred())
            goto Done;
        newn = PyNumber_Rshift(n, thirtytwo);
        if (newn == NULL)
            goto Done;
        Py_DECREF(n);
        n = newn;
        if (keyused >= keymax) {
            unsigned long bigger = keymax << 1;
            if ((bigger >> 1) != keymax) {
                PyErr_NoMemory();
                goto Done;
            }
            key = (uint32_t *)PyMem_Realloc(key,
					    bigger * sizeof(*key));
            if (key == NULL)
                goto Done;
            keymax = bigger;
        }
        assert(keyused < keymax);
        key[keyused++] = chunk;
    }

    if (keyused == 0)
        key[keyused++] = 0UL;
    dsfmt_init_by_array(&self->dsfmt, (uint32_t *)key, keyused);
    Py_INCREF(Py_None);
    result = Py_None;
Done:
    Py_XDECREF(masklower);
    Py_XDECREF(thirtytwo);
    Py_XDECREF(n);
    PyMem_Free(key);
    return result;
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

    /* Fill-out whole words, byte-by-byte to avoid endianness issues */
    for (i=0 ; i<bytes ; i+=4, k-=32) {
        r = dsfmt_genrand_uint32(&self->dsfmt);
        if (k < 32)
            r >>= (32 - k);
        bytearray[i+0] = (unsigned char)r;
        bytearray[i+1] = (unsigned char)(r >> 8);
        bytearray[i+2] = (unsigned char)(r >> 16);
        bytearray[i+3] = (unsigned char)(r >> 24);
    }

    /* little endian order to match bytearray assignment order */
    result = _PyLong_FromByteArray(bytearray, bytes, 1, 0);
    PyMem_Free(bytearray);
    return result;
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

RANDOM_METHODS_STRUCT();

RANDOM_CLASS_DOC(MODULE_NAME);

RANDOM_OBJECT_STRUCT(MODULE_NAME);

RANDOM_MODULE_DOC(MODULE_NAME);

RANDOM_MODULE_STRUCT(MODULE_NAME);

RANDOM_MODULE_INIT2(MODULE_NAME)
