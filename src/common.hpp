#pragma once

#include <cmath>

#define EPSILON (1e-8)
#define CLOSE_TO(a, b) (abs(a - b) <= EPSILON)

// In range, inclusive
#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))
