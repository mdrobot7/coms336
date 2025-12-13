#include "boundingBox.hpp"
#include "common.hpp"

#include <cmath>
#include <limits>

BoundingBox::BoundingBox()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            mIntersections[i][j] = 0;
        }
    }
}

BoundingBox::BoundingBox(double minX, double maxX, double minY, double maxY, double minZ, double maxZ)
{
    mIntersections[V_X][0] = minX - sPadding;
    mIntersections[V_X][1] = maxX + sPadding;
    mIntersections[V_Y][0] = minY - sPadding;
    mIntersections[V_Y][1] = maxY + sPadding;
    mIntersections[V_Z][0] = minZ - sPadding;
    mIntersections[V_Z][1] = maxZ + sPadding;
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

BoundingBox &BoundingBox::merge(const BoundingBox &other)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; i < 2; i++)
        {
            // Make as large a box as needed to hold both boxes
            if (abs(mIntersections[i][j]) > abs(other.mIntersections[i][j]))
            {
                mIntersections[i][j] = other.mIntersections[i][j];
            }
        }
    }
}

int BoundingBox::largestAxis()
{
    double x = mIntersections[V_X][1] - mIntersections[V_X][0];
    double y = mIntersections[V_Y][1] - mIntersections[V_Y][0];
    double z = mIntersections[V_Z][1] - mIntersections[V_Z][0];
    if (x > y && x > z)
    {
        return V_X;
    }
    if (y > z)
    {
        return V_Y;
    }
    return V_Z;
}

bool BoundingBox::compare(const BoundingBox &a, const BoundingBox &b, int axis)
{
    return a.mIntersections[axis][0] < b.mIntersections[axis][0];
}

bool BoundingBox::compare_x(const BoundingBox &a, const BoundingBox &b)
{
    compare(a, b, V_X);
}

bool BoundingBox::compare_y(const BoundingBox &a, const BoundingBox &b)
{
    compare(a, b, V_Y);
}

bool BoundingBox::compare_z(const BoundingBox &a, const BoundingBox &b)
{
    compare(a, b, V_Z);
}