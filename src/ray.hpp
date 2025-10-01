#pragma once
#include "matrix.hpp"

class Ray
{
public:
    vector_t mOrigin;
    vector_t mDir; // Normalized!

    color_t mColor;

    Ray();
    Ray(vector_t origin, vector_t dir);

    void addCollision(color_t color);
};