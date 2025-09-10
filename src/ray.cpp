#include "ray.hpp"

Ray::Ray() {}

Ray::Ray(vector_t origin, vector_t dir)
{
    mOrigin = origin;
    mDir = dir;
}