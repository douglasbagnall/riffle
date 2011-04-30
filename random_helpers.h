#if ! HAVE_RANDOM_HELPERS
#define HAVE_RANDOM_HELPERS 1
#include "Python.h"
#include <time.h>               /* for seeding to current time */
#include <stdio.h>              /* for opening urandom */

#include "misc.h"
#include "sha1.h"

/* Initialise a range of memory to a pseudo random state, using sha1 in a kind
   of CTR mode.  This is used for initialising various generators.*/

UNUSED static void
initialise_state(u8 *output, size_t outlen, u8 *input, size_t inlen){
    moz_SHA_CTX ctx;
    unsigned char hashout[20];
    size_t remaining = outlen;
    u32 ctr = 0;
    moz_SHA1_Init(&ctx);
    for(;;){
	moz_SHA1_Update(&ctx, input, inlen);
	moz_SHA1_Update(&ctx, &ctr, sizeof(u32));
	moz_SHA1_Final(hashout, &ctx);
	if (remaining < 20){
	    memcpy(output, hashout, remaining);
	    return;
	}
	memcpy(output, hashout, 20);
	output += 20;
	remaining -= 20;
    }
}

/* randomise from urandom if possible.
   Returns -1 on failure, 0 on success.
 */
UNUSED static int
urandomise_state(u8 *state, size_t len){
    FILE * fd = fopen("/dev/urandom", "r");
    if (fd == NULL){
	return -1;
    }
    size_t got = fread(state, 1, len, fd);/*ought to loop, probably*/
    if (got != len){
	fclose(fd);
	return -1;
    }
    fclose(fd);
    return 0;
}

/* extract a seed from a python object.
   If it is NULL or None, use a random seed.
   If it is a buffer (e.g. a string), hash it with sha1.
   Otherwise, use the python hash.
 */

UNUSED static int
extract_seed(PyObject *arg, u8 *seed, size_t seed_len){
    memset(seed, '#', seed_len);
    if (arg == NULL || arg == Py_None) {
	/* try first with urandom, fall back to time */
	if (urandomise_state(seed, seed_len) != 0){
	    time_t now;
	    time(&now);
	    snprintf((char *)seed, seed_len, "%lx%p%p", now, &arg, &now);
	}
    }
    else if (PyObject_CheckReadBuffer(arg)){
	const void *buffer;
	Py_ssize_t buffer_len;
	if (PyObject_AsReadBuffer(arg, &buffer, &buffer_len)){
	    return -1;
	}
	initialise_state(seed, seed_len, (u8*)buffer, buffer_len);
    }
    else {
	/*use python hash. it would be possible, but perhaps surprising to
	  use the string representation */
	long hash = PyObject_Hash(arg);
	snprintf((char *)seed, seed_len, "%ld", hash);
    }
    return 0;
}

#define RANDOM_DUMMY_STATE_SETTERS() \
static PyObject *					\
random_getstate(RandomObject *self)			\
{							\
    return NULL;					\
}							\
static PyObject *					\
random_setstate(RandomObject *self, PyObject *state)		\
{								\
    return NULL;						\
}								\

#define RANDOM_CLASS_NEW() static PyObject * \
random_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {\
    RandomObject *self;\
    PyObject *tmp;\
    if (type == &Random_Type && !_PyArg_NoKeywords("Random()", kwds))\
        return NULL;\
    self = (RandomObject *)type->tp_alloc(type, 0);\
    if (self == NULL)\
        return NULL;\
    tmp = random_seed(self, args);\
    if (tmp == NULL) {\
        Py_DECREF(self);\
        return NULL;\
    }\
    Py_DECREF(tmp);\
    return (PyObject *)self;\
}


#define RANDOM_METHODS_STRUCT() static PyMethodDef random_methods[] = {	\
    {"random",          (PyCFunction)random_random,  METH_NOARGS,\
        PyDoc_STR("random() -> x in the interval [0, 1).")},\
    {"seed",            (PyCFunction)random_seed,  METH_VARARGS,\
        PyDoc_STR("seed([n]) -> None.  Defaults to current time.")},\
    {"getstate",        (PyCFunction)random_getstate,  METH_NOARGS,\
        PyDoc_STR("getstate() -> tuple containing the current state.")},\
    {"setstate",          (PyCFunction)random_setstate,  METH_O,\
        PyDoc_STR("setstate(state) -> None.  Restores generator state.")},\
    {"getrandbits",     (PyCFunction)random_getrandbits,  METH_VARARGS,\
        PyDoc_STR("getrandbits(k) -> x.  Generates a long int with "\
                  "k random bits.")},\
    {NULL,              NULL}           /* sentinel */\
}

#define RANDOM_METHODS_STRUCT_NO_GETRANDBITS() static PyMethodDef random_methods[] = {	\
    {"random",          (PyCFunction)random_random,  METH_NOARGS,\
        PyDoc_STR("random() -> x in the interval [0, 1).")},\
    {"seed",            (PyCFunction)random_seed,  METH_VARARGS,\
        PyDoc_STR("seed([n]) -> None.  Defaults to current time.")},\
    {"getstate",        (PyCFunction)random_getstate,  METH_NOARGS,\
        PyDoc_STR("getstate() -> tuple containing the current state.")},\
    {"setstate",          (PyCFunction)random_setstate,  METH_O,\
        PyDoc_STR("setstate(state) -> None.  Restores generator state.")},\
    {NULL,              NULL}           /* sentinel */\
}

#define RANDOM_CLASS_DOC(name) PyDoc_STRVAR(random_doc,			\
				       "Random() -> create a random number generator with its own internal state; uses " #name)

#define RANDOM_OBJECT_STRUCT(name) static PyTypeObject Random_Type = {	\
    PyVarObject_HEAD_INIT(NULL, 0)                               \
    #name ".Random",                   /*tp_name*/		 \
    sizeof(RandomObject),               /*tp_basicsize*/       \
    0,                                  /*tp_itemsize*/       \
    /* methods */       \
    0,                                  /*tp_dealloc*/       \
    0,                                  /*tp_print*/       \
    0,                                  /*tp_getattr*/       \
    0,                                  /*tp_setattr*/       \
    0,                                  /*tp_reserved*/       \
    0,                                  /*tp_repr*/       \
    0,                                  /*tp_as_number*/       \
    0,                                  /*tp_as_sequence*/       \
    0,                                  /*tp_as_mapping*/       \
    0,                                  /*tp_hash*/       \
    0,                                  /*tp_call*/       \
    0,                                  /*tp_str*/       \
    PyObject_GenericGetAttr,            /*tp_getattro*/       \
    0,                                  /*tp_setattro*/       \
    0,                                  /*tp_as_buffer*/       \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /*tp_flags*/       \
    random_doc,                         /*tp_doc*/       \
    0,                                  /*tp_traverse*/       \
    0,                                  /*tp_clear*/       \
    0,                                  /*tp_richcompare*/       \
    0,                                  /*tp_weaklistoffset*/       \
    0,                                  /*tp_iter*/       \
    0,                                  /*tp_iternext*/       \
    random_methods,                     /*tp_methods*/       \
    0,                                  /*tp_members*/       \
    0,                                  /*tp_getset*/       \
    0,                                  /*tp_base*/       \
    0,                                  /*tp_dict*/       \
    0,                                  /*tp_descr_get*/       \
    0,                                  /*tp_descr_set*/       \
    0,                                  /*tp_dictoffset*/       \
    0,                                  /*tp_init*/       \
    0,                                  /*tp_alloc*/       \
    random_new,                         /*tp_new*/       \
    PyObject_Free,                      /*tp_free*/       \
    0,                                  /*tp_is_gc*/       \
}

#define RANDOM_MODULE_DOC(name) PyDoc_STRVAR(module_doc, "Random numbers using " #name);

#define RANDOM_MODULE_STRUCT(name)      \
static struct PyModuleDef randommodule = {\
    PyModuleDef_HEAD_INIT,\
    #name,    \
    module_doc,\
    -1,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL\
}

#define RANDOM_MODULE_INIT(name) PyMODINIT_FUNC	\
PyInit_ ## name  (void) \
{ \
    PyObject *m;\
    if (PyType_Ready(&Random_Type) < 0) \
        return NULL;\
    m = PyModule_Create(&randommodule);\
    if (m == NULL)\
        return NULL;\
    Py_INCREF(&Random_Type);\
    PyModule_AddObject(m, "Random", (PyObject *)&Random_Type);\
    return m;\
}

/*workaround for CPP expansion oddities*/
#define RANDOM_MODULE_INIT2(name) RANDOM_MODULE_INIT(name)


/* Add the ECRYPT_init call for ecrypt modules */
/* ECRYPT_init intialises tables common to all instances */
#define RANDOM_MODULE_INIT_ECRYPT(name) PyMODINIT_FUNC	\
PyInit_ ## name  (void) \
{ \
    PyObject *m;\
    if (PyType_Ready(&Random_Type) < 0) \
        return NULL;\
    m = PyModule_Create(&randommodule);\
    if (m == NULL)\
        return NULL;\
    Py_INCREF(&Random_Type);\
    PyModule_AddObject(m, "Random", (PyObject *)&Random_Type);\
    ECRYPT_init(); \
    return m;\
}

#define RANDOM_MODULE_INIT2_ECRYPT(name) RANDOM_MODULE_INIT_ECRYPT(name)



#endif
