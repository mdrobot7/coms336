#pragma once
#include "matrix.hpp"
#include "color.hpp"

class Ray
{
public:
    vector_t mOrigin;
    vector_t mDir; // Normalized!

    color_t mColor;

    double mIndexOfRefraction; // Index of refraction of the material we're currently in

    Ray();
    Ray(vector_t origin, vector_t dir);

    void addCollision(color_t color);
};