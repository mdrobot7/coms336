#include "bvh.hpp"

#include <algorithm>

BoundingVolumeHierarchy::BoundingVolumeHierarchy() {}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(std::vector<object::Primitive> &primitives)
{
    BoundingVolumeHierarchy(primitives, 0, primitives.size());
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(std::vector<object::Primitive> &primitives, size_t start, size_t end)
{
    // See ray tracing in one weekend, their implementation is pretty smart.
    // Just modifying it so it fits how I have the rest of my system set up.
    mPrimitive = NULL;
    mLeft = NULL;
    mRight = NULL;

    // Find the longest axis
    mBbox = BoundingBox();
    for (int i = start; i < end; i++)
    {
        mBbox.merge(primitives[i].boundingBox());
    }
    int axis = mBbox.largestAxis();
    int range = end - start;

    // Go down the tree, generating nodes and assigning bounding boxes
    if (range == 1)
    {
        mPrimitive = &primitives[start];
        mBbox = mPrimitive->boundingBox();
    }
    else if (range == 2)
    {
        mLeft = new BoundingVolumeHierarchy();
        mLeft->mPrimitive = &primitives[start];
        mLeft->mBbox = mLeft->mPrimitive->boundingBox();
        mRight = new BoundingVolumeHierarchy();
        mRight->mPrimitive = &primitives[start + 1];
        mRight->mBbox = mRight->mPrimitive->boundingBox();
    }
    else
    {
        // Recurse into each half of the BVH, split by object number along
        // the longest axis.

        // Can't pass args to std::sort callback, so we're doing this
        auto comparator = (axis == V_X)   ? BoundingBox::compare_x
                          : (axis == V_Y) ? BoundingBox::compare_y
                                          : BoundingBox::compare_z;
        std::sort(std::begin(primitives) + start, std::begin(primitives) + end, comparator);
        mLeft = new BoundingVolumeHierarchy(primitives, start, range / 2);
        mRight = new BoundingVolumeHierarchy(primitives, range / 2, end);
    }
}

BoundingVolumeHierarchy::~BoundingVolumeHierarchy()
{
    destroySubtree(this);
}

object::Primitive *BoundingVolumeHierarchy::intersects(Ray &r)
{
    if (mPrimitive)
    {
        // Reached the bottom of the BVH
        return mPrimitive;
    }

    bool intLeft = mLeft->mBbox.intersectsBox(r);
    bool intRight = mRight->mBbox.intersectsBox(r);
    if (intLeft)
    {
        return mLeft->intersects(r);
    }
    if (intRight)
    {
        return mRight->intersects(r);
    }
    return NULL;
}

void BoundingVolumeHierarchy::destroySubtree(BoundingVolumeHierarchy *subtree)
{
    if (subtree->mLeft)
    {
        destroySubtree(subtree->mLeft);
    }
    if (subtree->mRight)
    {
        destroySubtree(subtree->mRight);
    }
    delete subtree;
}