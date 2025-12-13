#pragma once

#include "ray.hpp"

class BoundingBox
{
public:
    static constexpr double sPadding = 0.001; // Padding between the object and bounding box

    double mIntersections[3][2];

    BoundingBox();
    BoundingBox(double minX, double maxX, double minY, double maxY, double minZ, double maxZ);

    bool intersectsBox(Ray &r);

private:
    /**
     * @brief Calculate time in which the ray intersects a
     * particular point along a particular axis.
     */
    double intersectionTime(Ray &r, double val, int axis);
};