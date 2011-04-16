//#include "Python.h"

#ifndef INVISIBLE
#define INVISIBLE __attribute__ ((visibility("hidden")))
#else
#warning INVISIBLE is set
#endif
#ifndef UNUSED
#define UNUSED __attribute__ ((unused))
#else
#warning UNUSED is set
#endif

#ifndef ECRYPT_PORTABLE
/*ecrypt-portable also defines these. */
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
#endif

#define DSFMT_LOW_MASK  UINT64_C(0x000FFFFFFFFFFFFF)
#define DSFMT_HIGH_CONST UINT64_C(0x3FF0000000000000)
#define DSFMT_SR	12

#define debug(format, ...) fprintf (stderr, (format),## __VA_ARGS__); fflush(stderr)

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
