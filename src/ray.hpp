#pragma once
#include "vector.hpp"
#include "color.hpp"

class Ray
{
public:
    Vector mOrigin;
    Vector mDir; // Normalized!

    Color mColor;

    double mIndexOfRefraction; // Index of refraction of the material we're currently in

    Ray();
    Ray(Vector origin, Vector dir);

    void addCollision(Color color);
};