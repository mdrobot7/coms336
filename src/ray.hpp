#pragma once
#include "matrix.hpp"

class Ray
{
public:
    vector_t mOrigin;
    vector_t mDir;

    Ray(vector_t origin, vector_t dir);
};