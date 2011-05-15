#include "Python.h"
#include "random_helpers.h"
#include "ccan/isaac/isaac64.c"

#define SEED_BYTES 20

typedef struct {
    PyObject_HEAD
    struct isaac64_ctx context;
} RandomObject;

static inline double
rng_double(RandomObject *self)
{
#if 0
    return isaac64_next_double(&self->context);
#else
    return dsfmt_int64_to_double(isaac64_next_uint64(&self->context));
#endif
}

static inline void
rng_seed(RandomObject *self, u8* seed, size_t len)
{
    isaac64_init(&self->context, seed, len);
}
