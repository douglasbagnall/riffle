/* Copyright 2011 Douglas Bagnall <douglas@paradise.net.nz> MIT License
 *
 * Part of Riffle, a collection of random number generators
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
#if ! HAVE_MISC_H
#define HAVE_MISC_H 1

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

#define FORCE_INLINE inline __attribute__((always_inline))

#if 0
#define	INLINE FORCE_INLINE
#else
#define	INLINE inline
#endif

#include <stdint.h>

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

#define DSFMT_INT64_TO_DOUBLE(x) do {(x) &= DSFMT_LOW_MASK; (x) |= DSFMT_HIGH_CONST;} while (0)

static inline double
dsfmt_int64_to_double(u64 raw){
    union{
	u64 i;
	double d;
    } x;
    x.i = raw;
    DSFMT_INT64_TO_DOUBLE(x.i);
    return x.d - 1.0;
}


#include <stdio.h>
#define debug(format, ...) fprintf (stderr, (format),## __VA_ARGS__); fflush(stderr)


/* Seeding routine using LCG from Mersenne Twister */
static inline void seed_from_uint(u8*seed, u32 len, u32 s){
    u32 i;
    if (s == 0)
	s = 4357;
    for (i = 0; i < len; i++){
        s = ((69069 * s) + 1);
        seed[i] = (s >> 24) ^ (s >> 16) ^ (s >> 8);
    }
}


#define ONE_RESCUE_ROUND(i)do{				\
    a = buffer[i];					\
    b <<= 12;						\
    b ^= a;						\
    a >>= 12;						\
    buffer[i] = a | DSFMT_HIGH_CONST;			\
    } while(0)


static inline void doubleise_u64_buffer_with_rescuees(u64 *restrict buffer,
						      int count,
						      u64 *restrict rescuees){
    u64 a, b = 0;
    int i, j = 0;
    for (i = 0; i < count; i += 5, j++){
	ONE_RESCUE_ROUND(i);
	ONE_RESCUE_ROUND(i + 1);
	ONE_RESCUE_ROUND(i + 2);
	ONE_RESCUE_ROUND(i + 3);
	ONE_RESCUE_ROUND(i + 4);
	DSFMT_INT64_TO_DOUBLE(b);
	rescuees[j] = b;
    }
}


#define  COERCE_DSFMT 0
#define  COERCE_LDEXP 1
#define  COERCE_MUL   2

#if DOUBLE_COERCION == COERCE_DSFMT
static inline void doubleise_u64_buffer(u64 *restrict buffer,
					int count){
    int i;
    for (i = 0; i < count; i++){
	DSFMT_INT64_TO_DOUBLE(buffer[i]);
    }
}
#elif DOUBLE_COERCION == COERCE_LDEXP
static inline void doubleise_u64_buffer(u64 *restrict buffer,
					int count){
    int i;
    double *d = (double *)buffer;
    for (i = 0; i < count; i++){
	d[i] = ldexp((double)buffer[i], -64);
    }
}
#elif DOUBLE_COERCION == COERCE_MUL
#define U64_TO_DOUBLE (1.0 / 18446744073709551616.0)
static inline void doubleise_u64_buffer(u64 *restrict buffer,
					int count){
    int i;
    double *d = (double *)buffer;
    for (i = 0; i < count; i++){
	d[i] = buffer[i] * U64_TO_DOUBLE;
    }
}
#endif


#endif
