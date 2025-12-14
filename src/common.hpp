#pragma once

#include <cmath>
#include <cstdlib>

#define EPSILON (1e-8)
#define CLOSE_TO(a, b) (std::abs(a - b) <= EPSILON)

// In range, inclusive
#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))

#define MAX(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

// Clamp value to within the range [lower, upper], saturating at
// the bounds if necessary.
#define CLAMP(val, lower, upper) \
  (({ __typeof__ (val) _val = (val); \
       __typeof__ (lower) _lower = (lower); \
       __typeof__ (upper) _upper = (upper); \
     (MIN(MAX(_val, _lower), _upper)); }))

/**
 * @brief Returns a random double in range [0, 1)
 *
 * @return double
 */
static inline double randomDouble()
{
  return rand() / (RAND_MAX + 1.0);
}