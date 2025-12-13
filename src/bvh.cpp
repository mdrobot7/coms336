#include "bvh.hpp"

BoundingVolumeHierarchy::BoundingVolumeHierarchy() {}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(object::Primitive primitives[]) {}

object::Primitive *BoundingVolumeHierarchy::intersects(Ray &r)
{
    if (mPrimitive)
    {
        // Reached the bottom of the BVH
        return mPrimitive;
    }

    bool intLeft = left->bbox.intersectsBox(r);
    bool intRight = left->bbox.intersectsBox(r);
    if (intLeft)
    {
        return left->intersects(r);
    }
    if (intRight)
    {
        return right->intersects(r);
    }
    return NULL;
}