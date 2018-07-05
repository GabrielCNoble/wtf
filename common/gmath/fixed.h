#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

typedef int32_t fixed_t;

#define FRAC_BITS 0x03ff
#define FRAC_BITS_SHIFT (1<<10)

fixed_t FixedFromFloat(float value);

float FloatFromFixed(fixed_t);

fixed_t FixedAdd(fixed_t a, fixed_t b);

fixed_t FixedSub(fixed_t a, fixed_t b);

fixed_t FixedMul(fixed_t a, fixed_t b);

fixed_t FixedDiv(fixed_t a, fixed_t b);


#endif
