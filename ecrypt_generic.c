/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * A Riffle wrapper for eSTREAM synchronous stream ciphers.  To use this on a
 * new cipher, you need to do this:
 *
 * 1. Copy the relevant files into a directory with the name you want for the
 *    eventual python module (so, no hyphens, dots, etc, which break python
 *    syntax).  There is always a <something>.c file and ecrypt-sync.h, and
 *    sometimes there's more.
 *
 * 2. Rename the main C file after the module.  That is, if your module will
 *    be foo, you directory is foo, and the C file is foo/foo.c (vs, say,
 *    foo-128.c).
 *
 * 3. Make sure the file has an ECRYPT_keystream_bytes function (as defined in
 *    ecrypt-sync.h).  Some ciphers have this already.  Some (e.g. DJB's ones)
 *    have a suboptimal implementation.  For most it is quite simple to write:
 *    their ECRYPT_encrypt_bytes function generate keystream which is xored
 *    with the input.  If you start in the middle and change the bit that goes
 *    'output[i] = input[i] ^ keystream[i]' to 'output[i] = keystream[i]',
 *    then work outwards deleting unused variables, you have it.  You can also
 *    skip the special case loop for finishing odd-sized chunks -- assume the
 *    wrapper always wants at least 32 bytes at a time.
 *
 * 4. Create a patch of your changes (if any).  Actually, the patch should
 *    apply to the C file in its original name, so steps 3 and 4 really come
 *    before 2.  But the patch file is optional -- you don't need it to get
 *    things working -- so you'll end up doing it this way and munging the
 *    patch afterwards.  The patch file goes in the project root directory,
 *    and is only really necessary if you want the cipher to be downloaded on
 *    demand from the ecrypt website.
 *
 * 5. Add your cipher to the ESTREAM_DATA variable in the Makefile.  This
 *    consists of 5 colon-separated fields.  First is the name of the
 *    directory you made in step 1, and is used as a stem for various files.
 *    The second is a one word summary of its license, either "free",
 *    "unknown", "mixed", or "non-free".  The only distinction that matters so
 *    far is "free" vs anything else: free modules always get built by "make
 *    all", while the others don't unless the code is there.  To fetch those
 *    ones, you need to use "make everything".  The third field is the
 *    filename of the patch (if any).  The fourth field is the name of the
 *    file that needs to be renamed to <stem>.c (in the example above,
 *    "foo-128.c").  Then last is path to the original directory in the
 *    eSTREAM subversion repository, treating /trunk/ as root. See here:
 *    http://www.ecrypt.eu.org/stream/svn/viewcvs.cgi/ecrypt/trunk/ .  You
 *    don't need much of this if you don't intend the cipher to be downloaded
 *    on demand.  A minimal example is "chacha8:free:::", which assumes the
 *    contents of the chacha8 directory are already appropriately named and
 *    patched.
 *
 * 6. If you did do the patch thing, try moving the cipher's directory safely
 *    away, and see what happens with "make <stem>.so".  It ought to download
 *    a tarball, restore the directory, and apply your patches.
 *
 * This file was originally based on Python 3.1's original randommodule.c,
 * which was in turn based on Takuji Nishimura and Makoto Matsumoto's MT19937
 * code and adapted to Python by Raymond Hettinger.  No trace of MT19937
 * remains here, and most of the Python boilerplate has been moved to
 * macros in random_helpers.h.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided "as is", WITHOUT WARRANTY of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. in no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising from,
 * out of or in connection with the software or the use or other dealings in
 * the Software.
 */

#include "Python.h"

#include "ecrypt-sync.h"

#include "random_helpers.h"
#define RESCUE_BITS (DOUBLE_COERCION == COERCE_DSFMT)

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
    u8 seed[KEY_BYTES + IV_BYTES + 4];

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

