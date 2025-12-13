#include "boundingBox.hpp"
#include "common.hpp"

#include <limits>

BoundingBox::BoundingBox() {}

BoundingBox::BoundingBox(double minX, double maxX, double minY, double maxY, double minZ, double maxZ)
{
    mIntersections[0][0] = minX - sPadding;
    mIntersections[0][1] = maxX + sPadding;
    mIntersections[1][0] = minY - sPadding;
    mIntersections[1][1] = maxY + sPadding;
    mIntersections[2][0] = minZ - sPadding;
    mIntersections[2][1] = maxZ + sPadding;
}

bool BoundingBox::intersectsBox(Ray &r)
{
    double minMaxInt = std::numeric_limits<double>::infinity();
    double maxMinInt = -std::numeric_limits<double>::infinity();

    // Find the intersection time range for each axis and track the
    // max of the minInts and min of the maxInts.
    for (int i = 0; i < 3; i++)
    {
        double int0 = intersectionTime(r, mIntersections[i][0], i);
        double int1 = intersectionTime(r, mIntersections[i][1], i);
        double thisMinInt = MIN(int0, int1);
        double thisMaxInt = MAX(int0, int1);
        maxMinInt = thisMinInt > maxMinInt ? thisMinInt : maxMinInt;
        minMaxInt = thisMaxInt < minMaxInt ? thisMaxInt : minMaxInt;
    }

    // Check if the ranges overlap and that the intersection isn't behind us
    return maxMinInt < minMaxInt && minMaxInt > 0;
}

double BoundingBox::intersectionTime(Ray &r, double val, int axis)
{
    // Solve P = O + td for t
    return (val - r.mOrigin[axis]) / r.mDir[axis];
}