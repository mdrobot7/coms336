#include "bvh.hpp"

#include <algorithm>
#include <typeinfo>

BoundingVolumeHierarchy::BoundingVolumeHierarchy()
{
    mPrimitive = NULL;
    mLeft = NULL;
    mRight = NULL;
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(std::vector<std::unique_ptr<object::Primitive>> &primitives) : BoundingVolumeHierarchy(primitives, 0, primitives.size()) {}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(std::vector<std::unique_ptr<object::Primitive>> &primitives, size_t start, size_t end)
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
        mBbox.merge(primitives[i]->mBoundingBox);
    }
    int axis = mBbox.largestAxis();
    int range = end - start;

    // Go down the tree, generating nodes and assigning bounding boxes.
    // Skip the camera, it doesn't have a bounding box (it's still a Primitive
    // for json reasons).
    if (range == 1 && typeid(*primitives[start]) != typeid(object::Camera))
    {
        mPrimitive = primitives[start].get();
        mBbox = mPrimitive->mBoundingBox;
    }
    else if (range == 2)
    {
        if (typeid(*primitives[start]) != typeid(object::Camera))
        {
            mLeft = new BoundingVolumeHierarchy();
            mLeft->mPrimitive = primitives[start].get();
            mLeft->mBbox = mLeft->mPrimitive->mBoundingBox;
        }
        if (typeid(*primitives[start + 1]) != typeid(object::Camera))
        {
            mRight = new BoundingVolumeHierarchy();
            mRight->mPrimitive = primitives[start + 1].get();
            mRight->mBbox = mRight->mPrimitive->mBoundingBox;
        }
    }
    else
    {
        // Recurse into each half of the BVH, split by object number along
        // the longest axis.

        // Can't pass args to std::sort callback, so we're doing this
        auto comparator = (axis == V_X)   ? BoundingVolumeHierarchy::compare_x
                          : (axis == V_Y) ? BoundingVolumeHierarchy::compare_y
                                          : BoundingVolumeHierarchy::compare_z;

        std::sort(std::begin(primitives) + start, std::begin(primitives) + end, comparator);
        mLeft = new BoundingVolumeHierarchy(primitives, start, start + range / 2);
        mRight = new BoundingVolumeHierarchy(primitives, start + range / 2, end);
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

    // Traverse down, making sure to traverse the subtree with
    // the *closer* bounding box. Handles cases where the ray
    // hits both bounding box subtrees.
    double tLeft, tRight;
    bool intLeft = mLeft->mBbox.intersectsBox(r, tLeft);
    bool intRight = mRight->mBbox.intersectsBox(r, tRight);
    if (intLeft && tLeft < tRight)
    {
        return mLeft->intersects(r);
    }
    if (intRight && tRight < tLeft)
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
}

bool BoundingVolumeHierarchy::compare_x(const std::unique_ptr<object::Primitive> &a, const std::unique_ptr<object::Primitive> &b)
{
    return BoundingBox::compare(a->mBoundingBox, b->mBoundingBox, V_X);
}

bool BoundingVolumeHierarchy::compare_y(const std::unique_ptr<object::Primitive> &a, const std::unique_ptr<object::Primitive> &b)
{
    return BoundingBox::compare(a->mBoundingBox, b->mBoundingBox, V_Y);
}

bool BoundingVolumeHierarchy::compare_z(const std::unique_ptr<object::Primitive> &a, const std::unique_ptr<object::Primitive> &b)
{
    return BoundingBox::compare(a->mBoundingBox, b->mBoundingBox, V_Z);
}
