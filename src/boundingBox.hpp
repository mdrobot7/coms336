#pragma once

#include "ray.hpp"

class BoundingBox
{
public:
    static constexpr double sPadding = 0.001; // Padding between the object and bounding box

    double mIntersections[3][2];

    BoundingBox();
    BoundingBox(double minX, double maxX, double minY, double maxY, double minZ, double maxZ);

    /**
     * @brief Returns true if a ray intersects with this
     * bounding box. Also returns the time (t) that the
     * ray hits the box, or infinity if no boxes were hit.
     */
    bool intersectsBox(Ray &r, double &t);

    /**
     * @brief Merges another bounding box into this one.
     * Returns a reference to this.
     */
    BoundingBox &merge(const BoundingBox &other);

    /**
     * @brief Return the largest axis of this bounding box.
     */
    int largestAxis();

    /**
     * @brief Compare two bounding boxes along an axis. Return
     * true if a's min is less than b's min. False otherwise.
     */
    static bool compare(const BoundingBox &a, const BoundingBox &b, int axis);

private:
    /**
     * @brief Calculate time in which the ray intersects a
     * particular point along a particular axis.
     */
    double intersectionTime(Ray &r, double val, int axis);
};