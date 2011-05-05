/* 
 * Copyright (C) 2011 Douglas Bagnall
 *
 * Part of Riffle, a collection of random number generators
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 */

#include <config.h>
#include <stdlib.h>
#include <gsl/gsl_rng.h>

#include "ecrypt-sync.h"
#include "misc.h"

#ifndef MODULE_NAME
#error define MODULE_NAME and possibly BUFFER_U32, KEY_BYTES, and IV_BYTES
#endif
#ifndef BUFFER_U32
#define BUFFER_U32 64
#endif
#ifndef KEY_BYTES
#define KEY_BYTES (128 / 8)
#endif
#ifndef IV_BYTES
#define IV_BYTES (64 / 8)
#endif

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

#define __ECRYPT_STATE_T(name) name ## _state_t
#define _ECRYPT_STATE_T(name) __ECRYPT_STATE_T(name)
#define ecrypt_state_t _ECRYPT_STATE_T(MODULE_NAME)

typedef struct{
  ECRYPT_ctx ctx;
  u32 numbers[BUFFER_U32];
  u32 index;
} ecrypt_state_t;


static inline unsigned long int
rand_get(void *vstate)
{
  ecrypt_state_t *state = (ecrypt_state_t *) vstate;  
  if (state->index >= BUFFER_U32){
    ECRYPT_keystream_bytes(&state->ctx,
        (u8*)state->numbers,
        BUFFER_U32 * sizeof(state->numbers[0]));
    state->index = 0;
  }
  return state->numbers[state->index++];
}

static double
rand_get_double (void *vstate)
{
  u32 r = rand_get(vstate);
  /*put the 32 bits in the most significant significand bits */
  u64 x = ((u64)r << 20) | DSFMT_HIGH_CONST;
  return *(double *)&x - 1.0;
  //XXX or try ldexp((double)x, -63) -1
}


/*set the state (integer only), using an LCG
borrowed from mt19937 */
static void
rand_set (void *vstate, unsigned long int s)
{
  ecrypt_state_t *state = (ecrypt_state_t *) vstate;
  int i;
  if (s == 0)
    s = 4357;   /* the default seed is 4357, following MT */
  u8 seed[KEY_BYTES + IV_BYTES + 4];
  for (i = 0; i < KEY_BYTES + IV_BYTES; i++)
    {
      s = ((69069 * s) + 1);
      seed[i] = (s >> 24) ^ (s >> 16) ^ (s >> 8); 
    }
  ECRYPT_keysetup(&state->ctx, seed, KEY_BYTES * 8, IV_BYTES * 8);
  ECRYPT_ivsetup(&state->ctx, seed + KEY_BYTES);
  state->index = BUFFER_U32;
}

static const gsl_rng_type ecrypt_type =
{QUOTE(MODULE_NAME),            /* name */
 0xffffffffUL,                  /* RAND_MAX */
 0,                             /* RAND_MIN */
 sizeof (ecrypt_state_t),
 &rand_set,
 &rand_get,
 &rand_get_double};


#define __GSL_RNG_TYPE(name) gsl_rng_ ## name
#define _GSL_RNG_TYPE(name) __GSL_RNG_TYPE(name)
#define GSL_RNG_TYPE _GSL_RNG_TYPE(MODULE_NAME)

const gsl_rng_type *GSL_RNG_TYPE = &ecrypt_type;
