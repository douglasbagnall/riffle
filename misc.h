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

static inline void doubleise_u64_buffer(u64 *restrict buffer,
					int count){
    int i;
    for (i = 0; i < count; i++){
	DSFMT_INT64_TO_DOUBLE(buffer[i]);
    }
}

#endif
