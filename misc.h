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

#define debug(format, ...) fprintf (stderr, (format),## __VA_ARGS__); fflush(stderr)

#endif
