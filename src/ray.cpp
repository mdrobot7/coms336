#include "ray.hpp"
#include "color.hpp"

Ray::Ray() {}

Ray::Ray(vector_t origin, vector_t dir)
{
    mOrigin = origin;
    mDir = dir;
    mColor = color_t{1.0, 1.0, 1.0};
    mIndexOfRefraction = 1.0; // Air
}

void Ray::addCollision(color_t color)
{
    mColor = Color::attenuate(mColor, color);
}
